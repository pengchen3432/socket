#!/bin/sh
##
## monitor.sh Background process to monitor cpu, memory, network usage and send email notifications
##

# To retrieve wan interfaces
. /lib/functions/gs_platform.sh

# Mailer script
MAILER=/usr/sbin/email_template.sh

# JSON parser api
. /usr/share/libubox/jshn.sh

# Check in every 60 seconds
REFRESH_TIME=60

# The max time interval of data in controller's databases was 6mins .
CHECK_TUROUGHPUT_INTERVAL=360

# Try read database again interval
TRY_AGAIN_READ_DB_INTERVAL=20

# Device type
PRODUCT=$( cat /proc/gxp/dev_info/dev_alias )
MAC_WITH_DELIM=$( cat /proc/gxp/dev_info/dev_mac )

# Alert/Notification event
ALERT_MODULE="email/notification"
ALERT_PRIORITY=0
EVENT_TYPE="grandstream"

# Query result file
QUERY_RESULT_FILE="/tmp/query_result"

# To notify or not notify
notify_memory_usage=0
notify_find_rogueap=0
notify_wan0_usage=0
notify_wan1_usage=0
notify_password_change=0
notify_network_group_change=0
notify_firmware_upgrade=0
notify_ap_offline=0
notify_ap_throughput=0
notify_ssid_throughput=0

# Thresholds for notification
memory_usage_threshold=0
wan0_usage_threshold=0
wan1_usage_threshold=0
ap_throughput_threshold=0
ssid_throughput_threshold=0

# Simple logger
log() {
    echo $@ | logger -t notification
}

# get_ap_monitor_data <mac> <memory_usage_notified | throughput_notified | offline_notified | firmware_version>
get_ap_monitor_data() {
    local line result=0
    local match_str="ap.${1}.${2}"

    for line in ${monitor_data}; do
        local target=$( echo "${line}" | grep "${match_str}" )

        if [ -n "${target}" ]; then
            result=$( echo "${line}" | cut -d "=" -f 2 )
            break;
        fi
    done

    echo ${result}
}

# set_ap_monitor_data <mac> <memory_usage_notified | throughput_notified | offline_notified | firmware_version> <value>
set_ap_monitor_data() {
    echo "ap.${1}.${2}=${3}" >> ${tmp_data_file}
}

# get_throughput_data <mac or ssid_nam> 
get_throughput_data() {
    local line result=0
    local match_str="${1}"

    for line in ${throughput_data}; do
        local target=$( echo "${line}" | cut -d "=" -f 1 )

        if [ "${target}" = "${match_str}" ]; then
            result=$( echo "${line}" | cut -d "=" -f 2 )
            break;
        fi
    done

    echo ${result}
}
# Check and notify for memory usage, firmware upgrade, offline
# and throughput for each paired ap
check_ap_notify() {
    local devices device json_data

    json_data=$( ubus call controller.discovery get_paired_devices )
    [ "$?" != "0" ] && return

    json_load "${json_data}"
    json_select devices
    [ "$?" != "0" ] && return

    json_get_keys devices
    [ -z "$devices" ] && return

    for device in ${devices}; do
        local status mac version_firmware type ignore_offline_notification
        local total_mem used_mem usage_mem alert_msg device_name

        json_select $device
        json_get_var mac mac
        json_get_var status status
        json_get_var version_firmware version_firmware
        json_get_var type type
        json_get_var ignore_offline_notification ignore_offline_notification
        json_select memory
        json_get_var total_mem total
        json_get_var used_mem used
        json_select ..
        json_select ..

        mac_with_delim=$( echo "${mac:0:2}:${mac:2:2}:${mac:4:2}:${mac:6:2}:${mac:8:2}:${mac:10:2}" )
        device_name=$( uci get grandstream.${mac}.name 2>/dev/null )
        if [ "x$device_name" != "x" ]; then
            mac_with_delim="$mac_with_delim ($device_name)"
        fi

        if [ "$total_mem" != "0" ]; then
            usage_mem=$( echo "${used_mem} ${total_mem}" | awk '{printf("%d",$1/$2*100)}' )
        fi

        if [ "${notify_memory_usage}" = "1" ] && [ "$total_mem" != "0" ]; then
            local memory_notified=$( get_ap_monitor_data "${mac}" "memory_usage_notified" )

            if [ "${usage_mem}" -gt "${memory_usage_threshold}" ]; then
                if [ "${memory_notified}" = "0" ]; then
                    log "AP ${mac_with_delim} memory usage %${usage_mem} exceeded threshold, sending email..."
                    echo "Access Point: ${mac_with_delim} memory usage is more than ${memory_usage_threshold}%." >> ${MAIL_BODY}
                    should_send_mail=1
                    alert_msg="Access Point: ${mac_with_delim} memory usage is more than ${memory_usage_threshold}%."
                    ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                fi
                set_ap_monitor_data "${mac}" "memory_usage_notified" "1"
            else
                if [ "${memory_notified}" = "1" ]; then
                    log "AP ${mac_with_delim} memory usage %${usage_mem} resturned below threshold, sending email..."
                    echo "Access Point: ${mac_with_delim} memory usage is less than ${memory_usage_threshold}%." >> ${MAIL_BODY}
                    should_send_mail=1
                    alert_msg="Access Point: ${mac_with_delim} memory usage is less than ${memory_usage_threshold}%."
                    ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                fi
                set_ap_monitor_data "${mac}" "memory_usage_notified" "0"
            fi
        fi

        if [ "${notify_firmware_upgrade}" = "1" ]; then
            local version=$( get_ap_monitor_data "${mac}" "firmware_version" )

            if [ "${version}" != "0" ] && [ -n "${version_firmware}" ] && [ "${version_firmware}" != "${version}" ]; then
                log "AP ${mac_with_delim} firmware upgraded, sending email..."
                echo "Access Point: ${mac_with_delim} firmware has been upgraded from ${version} to ${version_firmware}." >> ${MAIL_BODY}
                should_send_mail=1
                alert_msg="Access Point: ${mac_with_delim} firmware has been upgraded from ${version} to ${version_firmware}."
                ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
            fi

            if [ -n "${version_firmware}" ]; then
                set_ap_monitor_data "${mac}" "firmware_version" "${version_firmware}"
            fi
        fi

        if [ "${notify_ap_offline}" = "1" ]; then
            local offline_notified=$( get_ap_monitor_data "${mac}" "offline_notified" )
            local notify_offline_times=$( get_ap_monitor_data "${mac}" "offline_times" )

            if [ "${status}" = "offline" -a "${offline_notified}" = "0" ]; then
                let notify_offline_times+=1
                if [ "${ignore_offline_notification}" = "0" -a $notify_offline_times -ge 5 ]; then
                    log "AP ${mac_with_delim} offline, sending email..."
                    echo "Access Point: ${mac_with_delim} is offline." >> ${MAIL_BODY}
                    should_send_mail=1
                    alert_msg="Access Point: ${mac_with_delim} is offline."
                    ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                    set_ap_monitor_data "${mac}" "offline_notified" "1"
                    set_ap_monitor_data "${mac}" "offline_times" "0"
                else
                    set_ap_monitor_data "${mac}" "offline_notified" "0"
                    set_ap_monitor_data "${mac}" "offline_times" "${notify_offline_times}"
                fi
            elif [ "${status}" = "offline" -a "${offline_notified}" = "1" ] ; then
                set_ap_monitor_data "${mac}" "offline_notified" "1"
                set_ap_monitor_data "${mac}" "offline_times" "0"
            elif [ "${status}" = "online" ]; then
                if [ "${offline_notified}" = "1" ]; then
                    log "AP ${mac_with_delim} online, sending email..."
                    echo "Access Point: ${mac_with_delim} is online." >> ${MAIL_BODY}
                    should_send_mail=1
                    alert_msg="Access Point: ${mac_with_delim} is online."
                    ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                fi
                set_ap_monitor_data "${mac}" "offline_notified" "0"
                set_ap_monitor_data "${mac}" "offline_times" "0"
            fi
        fi

        if [ "${notify_ap_throughput}" = "1" ]; then
            local throughput_notified=$( get_ap_monitor_data "${mac}" "throughput_notified" )

            if [ ${db_updated} = 1 ]; then
                local throughput

                throughput=$( get_throughput_data "${mac}" )
                if [ "${throughput}" -gt "${ap_throughput_threshold}" ]; then
                    if [ "${throughput_notified}" = "0" ]; then
                        log "AP ${mac_with_delim} throughput ${throughput}bps exceeded threshold, sending email..."
                        echo "Access Point: ${mac_with_delim} throughput is more than ${ori_ap_throughput_threshold}bps." >> ${MAIL_BODY}
                        should_send_mail=1
                        alert_msg="Access Point: ${mac_with_delim} throughput is more than ${ori_ap_throughput_threshold}bps."
                        ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                    fi
                    set_ap_monitor_data "${mac}" "throughput_notified" "1"
                else
                    if [ "${throughput_notified}" = "1" ]; then
                        log "AP ${mac_with_delim} throughput ${throughput}bps  resturned below threshold, sending email..."
                        echo "Access Point: ${mac_with_delim} throughput is less than ${ori_ap_throughput_threshold}bps." >> ${MAIL_BODY}
                        should_send_mail=1
                        alert_msg="Access Point: ${mac_with_delim} throughput is less than ${ori_ap_throughput_threshold}bps."
                        ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                    fi
                    set_ap_monitor_data "${mac}" "throughput_notified" "0"
                fi
            else
                set_ap_monitor_data "${mac}" "throughput_notified" "${throughput_notified}"
            fi
        fi

        if [ "${notify_find_rogueap}" = "1" ]; then
            local msg=0
            local send_msg=0
            local local_mac
            ret=`cat ${ROGUEAP_EMAIL_SEND_PATH} | grep "update=1"`
            local_mac=`cat /proc/gxp/dev_info/dev_mac`

            if [ -n "$ret" ]; then
                should_send_mail=1
                echo -e "$local_mac find rogue AP.\n" > ${MAIL_BODY}
                msg=`cat ${ROGUEAP_EMAIL_SEND_PATH} | grep Untrusted | cut -d '=' -f 2`
                if [ -n "$msg" ]; then
                    send_msg=`echo Untrusted AP:$msg`
                    echo -e "$send_msg\n" >> ${MAIL_BODY}
                fi
                msg=`cat ${ROGUEAP_EMAIL_SEND_PATH} | grep authentication | cut -d '=' -f 2`
                if [ -n "$msg" ]; then
                    send_msg=`echo Illegal access without authentication:$msg`
                    echo -e "$send_msg\n" >> ${MAIL_BODY}
                fi
                msg=`cat ${ROGUEAP_EMAIL_SEND_PATH} | grep access | grep -v authentication | cut -d '=' -f 2`
                if [ -n "$msg" ]; then
                    send_msg=`echo Illegal access:$msg`
                    echo -e "$send_msg\n" >> ${MAIL_BODY}
                fi
                msg=`cat ${ROGUEAP_EMAIL_SEND_PATH} | grep Spoofing | cut -d '=' -f 2`
                if [ -n "$msg" ]; then
                    send_msg=`echo Spoofing SSID:$msg`
                    echo -e "$send_msg\n" >> ${MAIL_BODY}
                fi

                echo "update=0" > ${ROGUEAP_EMAIL_SEND_PATH}
            fi
        fi
    done
}

# get_ssid_monitor_data <ssid_name> <throughput_notified>
get_ssid_monitor_data() {
    local line result=0
    local match_str="ssid.${1}.${2}"

    for line in ${monitor_data}; do
        local target=$( echo "${line}" | grep "${match_str}" )

        if [ -n "${target}" ]; then
            result=$( echo "${line}" | cut -d "=" -f 2 )
            break;
        fi
    done

    echo ${result}
}

# set_ssid_monitor_data <ssid_name> <throughput_notified> <value>
set_ssid_monitor_data() {
    echo "ssid.${1}.${2}=${3}" >> ${tmp_data_file}
}

# Check and notify for memory usage, firmware upgrade, offline
# and throughput for each paired ap
check_ssid_notify() {
    local ssid ssids json_data

    json_data=$( ubus call controller.config config_get_ssids )
    [ "$?" != "0" ] && return

    json_load "${json_data}"
    json_select additional_ssids
    [ "$?" != "0" ] && return

    json_get_keys ssids
    [ -z "${ssids}" ] && return

    for ssid in ${ssids}; do
        local ssid_name alert_msg

        json_select ${ssid}
        json_get_var ssid_name ssid
        json_select ..

        if [ "${notify_ssid_throughput}" = "1" ]; then
            local throughput_notified=$( get_ssid_monitor_data "${ssid_name}" "throughput_notified" )

            if [ ${db_updated} = 1 ]; then
                local throughput

                throughput=$( get_throughput_data "${ssid_name}" )
                if [ "${throughput}" -gt "${ssid_throughput_threshold}" ]; then
                    if [ "${throughput_notified}" = "0" ]; then
                        log "SSID throughput ${throughput}bps exceeded threshold, sending email..."
                        echo "SSID: ${ssid_name} throughput is more than ${ori_ssid_throughput_threshold}bps." >> ${MAIL_BODY}
                        should_send_mail=1
                        alert_msg="SSID: ${ssid_name} throughput is more than ${ori_ssid_throughput_threshold}bps."
                        ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                    fi
                    set_ssid_monitor_data "${ssid_name}" "throughput_notified" "1"
                else
                    if [ "${throughput_notified}" = "1" ]; then
                        log "SSID throughput ${throughput}bps  resturned below threshold, sending email..."
                        echo "SSID: ${ssid_name} throughput is less than ${ori_ssid_throughput_threshold}bps." >> ${MAIL_BODY}
                        should_send_mail=1
                        alert_msg="SSID ${ssid_name} throughput has returned below threshold"
                        ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
                    fi
                    set_ssid_monitor_data "${ssid_name}" "throughput_notified" "0"
                fi
            else
                set_ssid_monitor_data "${ssid_name}" "throughput_notified" "${throughput_notified}"
            fi
        fi
    done
}

# get_ssid_monitor_data <admin_password | network_zones | wan0_usage_notified | wan1_usage_notified>
get_other_monitor_data() {
    local line result=0
    local match_str="${1}"

    for line in ${monitor_data}; do
        local target=$( echo "${line}" | grep "${match_str}" )

        if [ -n "${target}" ]; then
            result=$( echo "${line}" | cut -d "=" -f 2 )
            break;
        fi
    done

    echo ${result}
}

# set_ap_monitor_data <admin_password | network_zones | wan0_usage_notified | wan1_usage_notified> <value>
set_other_monitor_data() {
    echo "${1}=${2}" >> ${tmp_data_file}
}

# Calculate wan usage

# As the connections are full duplex, one capacity will work for tx and rx
wan0_capacity=0
wan1_capacity=0

prev_wan0_tx=0
prev_wan0_rx=0
prev_wan1_tx=0
prev_wan1_rx=0

# Gets the capacity of wan interfaces. Capacity is set to 0 if wan interface is disabled

get_wan_capacity() {
    local wan0_interface
    local wan1_interface
    wan0_interface=$( uci get network.wan0.ifname )
    wan0_capacity=$( cat /sys/class/net/${wan0_interface}/speed )
    # Convert mbit/sec to byte/sec
    if [ -z "${wan0_capacity}" ]; then
        #log "wan1 disabled ..."
        wan0_capcacity=0
    else
        wan0_capacity=$(( $wan0_capacity * 1024 * 1024 / 8 ))
    fi

    wan1_interface=$( uci get network.wan1.ifname )
    wan1_capacity=$( cat /sys/class/net/${wan1_interface}/speed )
    if [ -z "${wan1_capacity}" ]; then
        #log "wan2 disabled ..."
        wan1_capcacity=0
    else
        wan1_capacity=$(( $wan1_capacity * 1024 * 1024 / 8 ))
    fi
}

first_iter_wan0=0

calc_wan0_usage() {
    local wan0_tx
    local wan0_rx
    local wan0_usage_notified=$( get_other_monitor_data "wan0_usage_notified" )
    local wan0_interface=$( uci get network.wan0.ifname )
    local alert_msg

    if [  "${first_iter_wan0}" = "0" ]; then
        prev_wan0_tx=$( cat /sys/class/net/${wan0_interface}/statistics/tx_bytes )
        prev_wan0_rx=$( cat /sys/class/net/${wan0_interface}/statistics/rx_bytes )
        first_iter_wan0=1
    fi

    wan0_tx=$( cat /sys/class/net/${wan0_interface}/statistics/tx_bytes )
    wan0_rx=$( cat /sys/class/net/${wan0_interface}/statistics/rx_bytes )
    wan0_usage_rate=$( echo "${wan0_tx} ${prev_wan0_tx} ${wan0_rx} ${prev_wan0_rx} ${wan0_capacity} ${REFRESH_TIME}" | awk '{usage_rate=($1-$2+$3-$4)/($5*$6)*100} END {printf "%d", usage_rate}' )
    log "wan0 usage rate: ${wan0_usage_rate}"

    if [ "${wan0_usage_rate}" -gt "${wan0_usage_threshold}" ]; then
        if [ "${wan0_usage_notified}" = "0" ]; then
            log "Wan0 usage exceed threshold, sending email..."
            echo "Device: ${MAC_WITH_DELIM} wan port 1 usage is more than ${wan0_usage_threshold}%." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="Device: ${MAC_WITH_DELIM} wan port 1 usage is more than ${wan0_usage_threshold}%."
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "wan0_usage_notified" "1"
    else
        if [ "${wan0_usage_notified}" = "1" ]; then
            log "Wan0 usage threshold returning to normal, send email..."
            echo "Device: ${MAC_WITH_DELIM} wan port 1 usage is less than ${wan0_usage_threshold}%." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="Device: ${MAC_WITH_DELIM} wan port 1 usage is less than ${wan0_usage_threshold}%."
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "wan0_usage_notified" "0"
    fi

    prev_wan0_tx=${wan0_tx}
    prev_wan0_rx=${wan0_rx}
}

first_iter_wan1=0

calc_wan1_usage() {
    local wan1_tx
    local wan1_rx
    local wan1_usage_notified=$( get_other_monitor_data "wan1_usage_notified" )
    local wan1_interface=$( uci get network.wan1.ifname )
    local alert_msg

    if [ "${first_iter_wan1}" = "0" ]; then
       prev_wan1_tx=$( cat /sys/class/net/${wan1_interface}/statistics/tx_bytes )
       prev_wan1_rx=$( cat /sys/class/net/${wan1_interface}/statistics/rx_bytes )
       first_iter_wan1=1
    fi

    wan1_tx=$( cat /sys/class/net/${wan1_interface}/statistics/tx_bytes )
    wan1_rx=$( cat /sys/class/net/${wan1_interface}/statistics/rx_bytes )
    wan1_usage_rate=$( echo "${wan1_tx} ${prev_wan1_tx} ${wan1_rx} ${prev_wan1_rx} ${wan1_capacity} ${REFRESH_TIME}" | awk '{usage_rate=($1-$2+$3-$4)/($5*$6)*100} END {printf "%d", usage_rate}' )
    log "wan1 usage rate: ${wan1_usage_rate}"
    if [ "${wan1_usage_rate}" -gt "${wan1_usage_threshold}" ]; then
        if [ "${wan1_usage_notified}" = "0" ]; then
            log "Wan1 usage exceed threshold, sending email..."
            echo "Device: ${MAC_WITH_DELIM} wan port 2 usage is less than %${wan1_usage_threshold}." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="Device: ${MAC_WITH_DELIM} wan port 2 usage is less than %${wan1_usage_threshold}."
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "wan1_usage_notified" "1"
    else
        if [ "${wan1_usage_notified}" = "1" ]; then
            log "Wan1 usage threshold returning to normal, send email..."
            echo "Device: ${MAC_WITH_DELIM} wan port 2 usage is less than %${wan1_usage_threshold}." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="Device: ${MAC_WITH_DELIM} wan port 2 usage has returned below threshold"
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "wan1_usage_notified" "0"
    fi

    prev_wan1_tx=${wan1_tx}
    prev_wan1_rx=${wan1_rx}
}

check_other_notify() {
    local alert_msg

    if [ "${notify_password_change}" = "1" ]; then
        local pass=$( uci get grandstream.general.admin_password )
        local admin_password=$( get_other_monitor_data "admin_password" )

        if [ "${admin_password}" != "0" ] && [ "${admin_password}" != "${pass}" ]; then
            log "Admin password change, sending email..."
            echo "System administrator password has been changed." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="System administrator password has been changed."
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "admin_password" "${pass}"
    fi

    if [ "${notify_network_group_change}" = "1" ]; then
        local new_num_network_zones=$( cat /etc/config/grandstream | grep 'config zone' | wc -l )
        local num_network_zones=$( get_other_monitor_data "network_zones" )
        
        if [ "${num_network_zones}" != "0" ] && [ "${num_network_zones}" != "${new_num_network_zones}" ]; then
            log "Network group change, sending email..."
            echo "Network group settings has been changed." >> ${MAIL_BODY}
            should_send_mail=1
            alert_msg="Network group settings has been changed."
            ubus send ${EVENT_TYPE} "{ \"msg\":\"${alert_msg}\", \"pri\":$ALERT_PRIORITY, \"module\":\"${ALERT_MODULE}\" }"
        fi
        set_other_monitor_data "network_zones" "${new_num_network_zones}"
    fi

    if [ "${PRODUCT}" = "GWN7000" ]; then
        # Checks if a wan interface is enabled while we're sleeping
        get_wan_capacity

        if [ "${notify_wan0_usage}" = "1" ] && [ "${wan0_capacity}" -gt "0" ]; then
            calc_wan0_usage
        fi

        if [ "${notify_wan1_usage}" = "1" ] && [ "${wan1_capacity}" -gt "0" ]; then 
            calc_wan1_usage
        fi
    fi
}

# File to store data
data_file=/data/monitor_data
tmp_data_file=/tmp/monitor_data
monitor_data=""
IGNORE_COUNT_FILE=/data/monitor_ignore_count

# Load state data from file
load_data() {

    if [ -f "${data_file}" ]; then
        monitor_data=$( cat ${data_file} )
    else
        touch ${data_file}
    fi

    if [ -f "${tmp_data_file}" ]; then
        rm ${tmp_data_file}
        touch ${tmp_data_file}
    fi
}

save_data() {
    cp -f ${tmp_data_file} ${data_file}
}

ori_ap_throughput_threshold=
ori_ssid_throughput_threshold=

init() {
    local str_len throughput_threshold

    log "Init!"
    notify_memory_usage=$( uci get notification.notification.notify_memory_usage )
    notify_password_change=$( uci get notification.notification.notify_password_change )
    notify_ap_throughput=$( uci get notification.notification.notify_ap_throughput )
    notify_ssid_throughput=$( uci get notification.notification.notify_ssid_throughput )
    notify_network_group_change=$( uci get notification.notification.notify_network_group_change )
    notify_firmware_upgrade=$( uci get notification.notification.notify_firmware_upgrade )
    notify_ap_offline=$( uci get notification.notification.notify_ap_offline )
    notify_find_rogueap=$( uci get notification.notification.notify_find_rogueap )

    memory_usage_threshold=$( uci get notification.notification.memory_usage_threshold )
    ap_throughput_threshold=$( uci get notification.notification.ap_throughput_threshold )
    ori_ap_throughput_threshold=${ap_throughput_threshold}
    if [ -n "$ap_throughput_threshold" ]; then
        local str_len=$((${#ap_throughput_threshold}-1))

        if [ "${ap_throughput_threshold:$str_len:1}" = 'M' ]; then
            throughput_threshold=$( echo "${ap_throughput_threshold}" | sed 's/.$//' )
            ap_throughput_threshold=$( expr ${throughput_threshold} \* 1000000 )
        elif [ "${ap_throughput_threshold:$str_len:1}" = 'K' ]; then
            throughput_threshold=$( echo "${ap_throughput_threshold}" | sed 's/.$//' )
            ap_throughput_threshold=$( expr ${throughput_threshold} \* 1000 )
        fi
    fi

    ssid_throughput_threshold=$( uci get notification.notification.ssid_throughput_threshold )
    ori_ssid_throughput_threshold=${ssid_throughput_threshold}
    if [ -n "$ssid_throughput_threshold" ]; then
        local str_len=$((${#ssid_throughput_threshold}-1))

        if [ "${ssid_throughput_threshold:$str_len:1}" = 'M' ]; then
            throughput_threshold=$( echo "${ssid_throughput_threshold}" | sed 's/.$//' )
            ssid_throughput_threshold=$( expr ${throughput_threshold} \* 1000000 )
        elif [ "${ssid_throughput_threshold:$str_len:1}" = 'K' ]; then
            throughput_threshold=$( echo "${ssid_throughput_threshold}" | sed 's/.$//' )
            ssid_throughput_threshold=$( expr ${throughput_threshold} \* 1000 )
        fi
    fi

    # WAN usage will only be calculated for gwn7000
    if [ "${PRODUCT}" = "GWN7000" ]; then
        notify_wan0_usage=$( uci get notification.notification.notify_wan0_usage )
        notify_wan1_usage=$( uci get notification.notification.notify_wan1_usage )
        wan0_usage_threshold=$( uci get notification.notification.wan0_usage_threshold )
        wan1_usage_threshold=$( uci get notification.notification.wan1_usage_threshold )
        get_wan_capacity
    fi
}

log "Startup ..."
enabled=$( uci get email.email.enable_notification )
if [ "${enabled}" = "0" ] || [ -z "${enabled}" ]; then
    log "Email notifications not enabled. Exiting ..."
    exit 0
fi

if [ "$( uci get controller.main.role )" != "master" ]; then
    log "We are not master. Exiting ..."
    exit 0
fi

init
INIT=0
THROUGHPUT_DATA_PATH="/tmp/throughput_data"
ROGUEAP_EMAIL_SEND_PATH="/tmp/rogueap_email_alarm"
throughput_data=""
MAIL_BODY="/tmp/mail_body"
should_send_mail=0

# main loop. will run indefinitely
while true
do
    # Waiting for network becomes ready
    if [ ${INIT} -eq 0 ]; then
        sleep 30
        INIT=1
    fi

    # Check permissions of /tmp/resolv.conf
    # Make it world readable if its not
    # This is a temporary fix. The file should be made world readable when created
    perm=$( stat -c %a /tmp/resolv.conf )
    if [ ${perm} != "644" ]; then
        chmod +r /tmp/resolv.conf
    fi

    # Load state data
    load_data

    if [ -f ${THROUGHPUT_DATA_PATH} ]; then
        db_updated=$( cat ${THROUGHPUT_DATA_PATH} | grep "update" | cut -d '=' -f 2)
        if [ "${db_updated}" = "1" ]; then
            throughput_data=$( cat ${THROUGHPUT_DATA_PATH} )
            echo "update=0" > ${THROUGHPUT_DATA_PATH}
        fi
    fi

    rm -f ${MAIL_BODY}
    touch ${MAIL_BODY}
    check_ap_notify
    check_ssid_notify
    check_other_notify

    # Save state data
    save_data

    # Check if we need to quit
    enabled=$( uci get email.email.enable_notification )
    if [ ${enabled} = "0" ]; then
        log "Email notifications disabled. Exiting ..."
        exit 0
    fi

    if [ ${should_send_mail} = 1 ]; then
        should_send_mail=0
        ${MAILER} "Alert Information"
    fi

    sleep ${REFRESH_TIME}
done
