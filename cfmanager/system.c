/****************************************************************************
* *
* * FILENAME:        $RCSfile: system.c,v $
* *
* * LAST REVISION:   $Revision: 1.0 $
* * LAST MODIFIED:   $Date: 2021/03/15
* *
* * DESCRIPTION:     xxxx feature: 
* *
* *
* * Copyright (c) 2021 by Grandstream Networks, Inc.
* * All rights reserved.
* *
* * This material is proprietary to Grandstream Networks, Inc. and,
* * in addition to the above mentioned Copyright, may be
* * subject to protection under other intellectual property
* * regimes, including patents, trade secrets, designs and/or
* *
* * Any use of this material for any purpose, except with an
* * express license from Grandstream Networks, Inc. is strictly
* * prohibited.
* *
* ***************************************************************************/

//=================
//  Includes
//=================
#include <libubox/vlist.h>
#include <uci.h>
#include "global.h"
#include "track.h"
#include "initd.h"
#include "utils.h"
#include "time.h"
#include "apply.h"
#include "system.h"
#include "cfparse.h"
#include "check.h"

//=================
//  Defines
//=================

//=================
//  Typedefs
//=================
enum {
    SYSTEM_HOSTNAME,
    SYSTEM_LOG_FILE,
    SYSTEM_LOG_SIZE,
    SYSTEM_LOGLEVEL,
    SYSTEM_TIMEZONE,
    SYSTEM_LOG_URI,
    SYSTEM_LOG_PRO,

    __SYSTEM_SET_MAX
};

//=================
//  Globals
//=================
struct vlist_tree basic_system_vltree;
struct zone_list timezones_name[] = {
    { "Etc/GMT+12",  "(UTC-12:00) International Date Line West" },
    { "Pacific/Midway",  "(UTC-11:00) Midway Islands" },
    { "Pacific/Honolulu",  "(UTC-10:00) Hawaii" },
    { "America/Anchorage",  "(UTC-09:00) Alaska" },
    { "America/Los_Angeles",  "(UTC-08:00) Pacific Time (US & Canada)" },
    { "America/Phoenix",  "(UTC-07:00) Arizona" },
    { "America/Chihuahua",  "(UTC-07:00) Chihuahua, La Paz, Mazatlan" },
    { "America/Denver",  "(UTC-07:00) Mountain Time (US & Canada)" },
    { "America/Tegucigalpa",  "(UTC-06:00) Central America" },
    { "America/Chicago",  "(UTC-06:00) Central Time (US & Canada)" },
    { "America/Mexico_City",  "(UTC-06:00) Guadalajara, Mexico City, Monterrey" },
    { "America/Regina",  "(UTC-06:00) Saskatchewan" },
    { "America/Bogota",  "(UTC-05:00) Bogota, Lima, Quito" },
    { "America/New_York",  "(UTC-05:00) Eastern Time (US & Canada)" },
    { "America/Indiana/Indianapolis",  "(UTC-05:00) Indiana (East)" },
    { "America/Caracas",  "(UTC-04:30) Caracas" },
    { "America/Asuncion",  "(UTC-04:00) Asuncion" },
    { "America/Halifax",  "(UTC-04:00) Atlantic Time (Canada)" },
    { "America/Cuiaba",  "(UTC-04:00) Cuiaba" },
    { "America/La_Paz",  "(UTC-04:00) Georgetown, La Paz, Manaus, San Juan" },
    { "America/St_Johns",  "(UTC-03:30) Newfoundland" },
    { "America/Sao_Paulo",  "(UTC-03:00) Brasilia, Sao Paulo" },
    { "America/Buenos_Aires",  "(UTC-03:00) Buenos Aires" },
    { "America/Cayenne",  "(UTC-03:00) Cayenne, Fortaleza, Salvador" },
    { "America/Godthab",  "(UTC-03:00) Greenland" },
    { "America/Montevideo",  "(UTC-03:00) Montevideo" },
    { "America/Santiago",  "(UTC-03:00) Santiago" },
    { "Atlantic/South_Georgia",  "(UTC-02:00) Mid-Atlantic" },
    { "Atlantic/Azores",  "(UTC-01:00) Azores" },
    { "Atlantic/Cape_Verde",  "(UTC-01:00) Cape Verde Is." },
    { "Africa/Casablanca",  "(UTC) Casablanca" },
    { "UTC",  "(UTC) Coordinated Universal Time" },
    { "Europe/London",  "(UTC) Dublin, Edinburgh, Lisbon, London" },
    { "Atlantic/Reykjavik",  "(UTC) Monrovia, Reykjavik" },
    { "Europe/Amsterdam",  "(UTC+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna" },
    { "Europe/Belgrade",  "(UTC+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague" },
    { "Europe/Brussels",  "(UTC+01:00) Brussels, Copenhagen, Madrid, Paris" },
    { "Europe/Sarajevo",  "(UTC+01:00) Sarajevo, Skopje, Warsaw, Zagreb" },
    { "Africa/Algiers",  "(UTC+01:00) West Central Africa" },
    { "Europe/Athens",  "(UTC+02:00) Athens, Bucharest" },
    { "Asia/Beirut",  "(UTC+02:00) Beirut" },
    { "Africa/Cairo",  "(UTC+02:00) Cairo" },
    { "Asia/Damascus",  "(UTC+02:00) Damascus" },
    { "Africa/Harare",  "(UTC+02:00) Harare, Pretoria" },
    { "Europe/Vilnius",  "(UTC+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius" },
    { "Europe/Istanbul",  "(UTC+02:00) Istanbul" },
    { "Asia/Jerusalem",  "(UTC+02:00) Jerusalem " },
    { "Asia/Amman",  "(UTC+03:00) Amman" },
    { "Asia/Baghdad",  "(UTC+03:00) Baghdad" },
    { "Europe/Minsk",  "(UTC+03:00) Kaliningrad, Minsk" },
    { "Asia/Kuwait",  "(UTC+03:00) Kuwait, Riyadh" },
    { "Europe/Moscow",  "(UTC+03:00) Moscow, St. Petersburg, Volgograd" },
    { "Africa/Nairobi",  "(UTC+03:00) Nairobi" },
    { "Asia/Tehran",  "(UTC+03:30) Tehran" },
    { "Asia/Muscat",  "(UTC+04:00) Abu Dhabi, Muscat" },
    { "Asia/Baku",  "(UTC+04:00) Baku" },
    { "Asia/Tbilisi",  "(UTC+04:00) Tbilisi" },
    { "Asia/Yerevan",  "(UTC+04:00) Yerevan" },
    { "Asia/Kabul",  "(UTC+04:30) Kabul" },
    { "Asia/Karachi",  "(UTC+05:00) Islamabad, Karachi" },
    { "Asia/Tashkent",  "(UTC+05:00) Tashkent" },
    { "Asia/Kolkata",  "(UTC+05:30) Chennai, Kolkata, Mumbai, New Delhi" },
    { "Asia/Colombo",  "(UTC+05:30) Sri Jayawardenepura" },
    { "Asia/Katmandu",  "(UTC+05:45) Kathmandu" },
    { "Asia/Dhaka",  "(UTC+06:00) Astana, Dhaka" },
    { "Asia/Yekaterinburg",  "(UTC+06:00) Ekaterinburg" },
    { "Asia/Rangoon",  "(UTC+06:30) Yangon (Rangoon)" },
    { "Asia/Bangkok",  "(UTC+07:00) Bangkok, Hanoi, Jakarta" },
    { "Asia/Novosibirsk",  "(UTC+07:00) Novosibirsk" },
    { "Asia/Hong_Kong",  "(UTC+08:00) Beijing, Chongqing, Hong Kong, Urumqi" },
    { "Asia/Krasnoyarsk",  "(UTC+08:00) Krasnoyarsk" },
    { "Asia/Kuala_Lumpur",  "(UTC+08:00) Kuala Lumpur, Singapore" },
    { "Australia/Perth",  "(UTC+08:00) Perth" },
    { "Asia/Taipei",  "(UTC+08:00) Taipei" },
    { "Asia/Ulaanbaatar",  "(UTC+08:00) Ulaanbaatar" },
    { "Asia/Irkutsk",  "(UTC+09:00) Irkutsk" },
    { "Asia/Tokyo",  "(UTC+09:00) Osaka, Sapporo, Tokyo" },
    { "Asia/Seoul",  "(UTC+09:00) Seoul" },
    { "Australia/Adelaide",  "(UTC+09:30) Adelaide" },
    { "Australia/Darwin",  "(UTC+09:30) Darwin" },
    { "Australia/Brisbane",  "(UTC+10:00) Brisbane" },
    { "Australia/Sydney",  "(UTC+10:00) Canberra, Melbourne, Sydney" },
    { "Pacific/Guam",  "(UTC+10:00) Guam, Port Moresby" },
    { "Australia/Hobart",  "(UTC+10:00) Hobart" },
    { "Asia/Yakutsk",  "(UTC+10:00) Yakutsk" },
    { "Pacific/Noumea",  "(UTC+11:00) Solomon Is., New Caledonia" },
    { "Asia/Vladivostok",  "(UTC+11:00) Vladivostok" },
    { "Pacific/Auckland",  "(UTC+12:00) Auckland, Wellington" },
    { "Pacific/Fiji",  "(UTC+12:00) Fiji" },
    { "Asia/Magadan",  "(UTC+12:00) Magadan" },
    { "Asia/Kamchatka",  "(UTC+12:00) Kamchatka" },
    { "Pacific/Tongatapu",  "(UTC+13:00) Nukualofa" }
};
const int timezones_name_num = sizeof( timezones_name )/sizeof( timezones_name[0] );

const char* timezone_values[] = {
    "GMT0",
    "GMT0",
    "EAT-3",
    "CET-1",
    "EAT-3",
    "GMT0",
    "WAT-1",
    "GMT0",
    "GMT0",
    "CAT-2",
    "WAT-1",
    "CAT-2",
    "EET-2",
    "<00>0<+01>,M5.3.0/2,M10.5.0/3", //"<+01>-1",Casablance timezone is not right and every year will change.
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "GMT0",
    "GMT0",
    "EAT-3",
    "EAT-3",
    "WAT-1",
    "<+01>-1",
    "GMT0",
    "CAT-2",
    "CAT-2",
    "SAST-2",
    "EAT-3",
    "EAT-3",
    "CAT-2",
    "CAT-2",
    "WAT-1",
    "WAT-1",
    "WAT-1",
    "GMT0",
    "WAT-1",
    "CAT-2",
    "CAT-2",
    "WAT-1",
    "CAT-2",
    "SAST-2",
    "SAST-2",
    "EAT-3",
    "GMT0",
    "EAT-3",
    "WAT-1",
    "WAT-1",
    "GMT0",
    "GMT0",
    "WAT-1",
    "GMT0",
    "EET-2",
    "CET-1",
    "CAT-2",
    "HST10HDT,M3.2.0,M11.1.0",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "AST4",
    "AST4",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "AST4",
    "<-04>4<-03>,M10.1.0/0,M3.4.0/0",
    "EST5",
    "<-03>3",
    "CST6CDT,M4.1.0,M10.5.0",
    "AST4",
    "<-03>3",
    "CST6",
    "AST4",
    "<-04>4",
    "<-05>5",
    "MST7MDT,M3.2.0,M11.1.0",
    "MST7MDT,M3.2.0,M11.1.0",
    "<-04>4",
    "EST5",
    "<-04>4",
    "<-03>3",
    "EST5",
    "CST6CDT,M3.2.0,M11.1.0",
    "MST7MDT,M4.1.0,M10.5.0",
    "CST6",
    "MST7",
    "<-04>4",
    "AST4",
    "GMT0",
    "MST7",
    "MST7",
    "MST7MDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "AST4",
    "MST7MDT,M3.2.0,M11.1.0",
    "<-05>5",
    "CST6",
    "MST7",
    "<-03>3",
    "AST4ADT,M3.2.0,M11.1.0",
    "AST4ADT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "AST4",
    "AST4",
    "CST6",
    "<-05>5",
    "<-04>4",
    "AST4ADT,M3.2.0,M11.1.0",
    "CST5CDT,M3.2.0/0,M11.1.0/1",
    "MST7",
    "EST5EDT,M3.2.0,M11.1.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "MST7MDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "AST4",
    "<-04>4",
    "<-05>5",
    "PST8PDT,M3.2.0,M11.1.0",
    "AST4",
    "<-03>3",
    "CST6",
    "<-04>4",
    "AST4",
    "AST4",
    "CST6CDT,M3.2.0,M11.1.0",
    "MST7MDT,M4.1.0,M10.5.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "CST6CDT,M4.1.0,M10.5.0",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "CST6CDT,M4.1.0,M10.5.0",
    "<-03>3<-02>,M3.2.0,M11.1.0",
    "AST4ADT,M3.2.0,M11.1.0",
    "CST6CDT,M4.1.0,M10.5.0",
    "<-03>3",
    "AST4",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "<-02>2",
    "CST6CDT,M3.2.0,M11.1.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "<-03>3<-02>,M3.5.0/-2,M10.5.0/-1",
    "MST7MDT,M3.2.0,M11.1.0",
    "EST5",
    "EST5EDT,M3.2.0,M11.1.0",
    "<-03>3",
    "MST7",
    "AST4",
    "EST5EDT,M3.2.0,M11.1.0",
    "<-04>4",
    "AST4",
    "<-03>3",
    "CST6CDT,M3.2.0,M11.1.0",
    "CST6CDT,M3.2.0,M11.1.0",
    "<-03>3",
    "CST6",
    "CST6CDT,M3.2.0,M11.1.0",
    "<-05>5",
    "<-03>3",
    "<-04>4<-03>,M9.1.6/24,M4.1.6/24",
    "AST4",
    "<-03>3",
    "<-01>1<+00>,M3.5.0/0,M10.5.0/1",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "AST4",
    "NST3:30NDT,M3.2.0,M11.1.0",
    "AST4",
    "AST4",
    "AST4",
    "AST4",
    "CST6",
    "CST6",
    "AST4ADT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "PST8PDT,M3.2.0,M11.1.0",
    "EST5EDT,M3.2.0,M11.1.0",
    "AST4",
    "PST8PDT,M3.2.0,M11.1.0",
    "MST7",
    "CST6CDT,M3.2.0,M11.1.0",
    "AKST9AKDT,M3.2.0,M11.1.0",
    "MST7MDT,M3.2.0,M11.1.0",
    "<+11>-11",
    "<+07>-7",
    "<+10>-10",
    "AEST-10AEDT,M10.1.0,M4.1.0/3",
    "<+05>-5",
    "NZST-12NZDT,M9.5.0,M4.1.0/3",
    "<-03>3",
    "<-03>3",
    "<+03>-3",
    "<+00>0<+02>-2,M3.5.0/1,M10.5.0/3",
    "<+06>-6",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "<+03>-3",
    "<+06>-6",
    "EET-2EEST,M3.5.4/24,M10.5.5/1",
    "<+12>-12",
    "<+05>-5",
    "<+05>-5",
    "<+05>-5",
    "<+05>-5",
    "<+03>-3",
    "<+03>-3",
    "<+04>-4",
    "<+07>-7",
    "<+07>-7",
    "EET-2EEST,M3.5.0/0,M10.5.0/0",
    "<+06>-6",
    "<+08>-8",
    "<+09>-9",
    "<+08>-8",
    "<+0530>-5:30",
    "EET-2EEST,M3.5.5/0,M10.5.5/0",
    "<+06>-6",
    "<+09>-9",
    "<+04>-4",
    "<+05>-5",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "EET-2EEST,M3.4.4/48,M10.4.4/49",
    "EET-2EEST,M3.4.4/48,M10.4.4/49",
    "<+07>-7",
    "HKT-8",
    "<+07>-7",
    "<+08>-8",
    "WIB-7",
    "WIT-9",
    "IST-2IDT,M3.5.6/2,M10.5.0/2", // OPENWRT: IST-2IDT,M3.4.4/26,M10.5.0 error
    "<+0430>-4:30",
    "<+12>-12",
    "PKT-5",
    "<+0545>-5:45",
    "<+09>-9",
    "IST-5:30",
    "<+07>-7",
    "<+08>-8",
    "<+08>-8",
    "<+03>-3",
    "CST-8",
    "<+11>-11",
    "WITA-8",
    "PST-8",
    "<+04>-4",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "<+07>-7",
    "<+07>-7",
    "<+06>-6",
    "<+05>-5",
    "<+07>-7",
    "WIB-7",
    "KST-9",
    "<+03>-3",
    "<+06>-6",
    "<+05>-5",
    "<+03>-3",
    "<+11>-11",
    "<+05>-5",
    "KST-9",
    "CST-8",
    "<+08>-8",
    "<+11>-11",
    "CST-8",
    "<+05>-5",
    "<+04>-4",
    "<+0330>-3:30<+0430>,J79/24,J263/24",
    "<+06>-6",
    "JST-9",
    "<+07>-7",
    "<+08>-8",
    "<+06>-6",
    "<+10>-10",
    "<+07>-7",
    "<+10>-10",
    "<+09>-9",
    "<+0630>-6:30",
    "<+05>-5",
    "<+04>-4",
    "<-01>1<+00>,M3.5.0/0,M10.5.0/1",
    "AST4ADT,M3.2.0,M11.1.0",
    "WET0WEST,M3.5.0/1,M10.5.0",
    "<-01>1",
    "WET0WEST,M3.5.0/1,M10.5.0",
    "WET0WEST,M3.5.0/1,M10.5.0",
    "GMT0",
    "<-02>2",
    "GMT0",
    "<-03>3",
    "ACST-9:30ACDT,M10.1.0,M4.1.0/3",
    "AEST-10",
    "ACST-9:30ACDT,M10.1.0,M4.1.0/3",
    "AEST-10AEDT,M10.1.0,M4.1.0/3",
    "ACST-9:30",
    "<+0845>-8:45",
    "AEST-10AEDT,M10.1.0,M4.1.0/3",
    "AEST-10",
    "<+1030>-10:30<+11>-11,M10.1.0,M4.1.0",
    "AEST-10AEDT,M10.1.0,M4.1.0/3",
    "AWST-8",
    "AEST-10AEDT,M10.1.0,M4.1.0/3",
    "GMT0",
    "<-01>1",
    "<-10>10",
    "<-11>11",
    "<-12>12",
    "<-02>2",
    "<-03>3",
    "<-04>4",
    "<-05>5",
    "<-06>6",
    "<-07>7",
    "<-08>8",
    "<-09>9",
    "<+01>-1",
    "<+10>-10",
    "<+11>-11",
    "<+12>-12",
    "<+13>-13",
    "<+14>-14",
    "<+02>-2",
    "<+03>-3",
    "<+04>-4",
    "<+05>-5",
    "<+06>-6",
    "<+07>-7",
    "<+08>-8",
    "<+09>-9",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "<+04>-4",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "IST-1GMT0,M10.5.0,M3.5.0/1",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "GMT0BST,M3.5.0/1,M10.5.0",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "GMT0BST,M3.5.0/1,M10.5.0",
    "<+03>-3",
    "GMT0BST,M3.5.0/1,M10.5.0",
    "EET-2",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "<+03>-3",
    "WET0WEST,M3.5.0/1,M10.5.0",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "GMT0BST,M3.5.0/1,M10.5.0",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "<+03>-3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "MSK-3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "<+04>-4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "<+04>-4",
    "MSK-3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "<+04>-4",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "<+04>-4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EET-2EEST,M3.5.0/3,M10.5.0/4",
    "CET-1CEST,M3.5.0,M10.5.0/3",
    "EAT-3",
    "<+06>-6",
    "<+07>-7",
    "<+0630>-6:30",
    "EAT-3",
    "<+05>-5",
    "<+04>-4",
    "<+05>-5",
    "<+04>-4",
    "EAT-3",
    "<+04>-4",
    "<+13>-13<+14>,M9.5.0/3,M4.1.0/4",
    "NZST-12NZDT,M9.5.0,M4.1.0/3",
    "<+11>-11",
    "<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45",
    "<+10>-10",
    "<-06>6<-05>,M9.1.6/22,M4.1.6/22",
    "<+11>-11",
    "<+13>-13",
    "<+13>-13",
    "<+12>-12", //OPENWRT: <+12>-12<+13>,M11.2.0,M1.2.3/99 is error
    "<+12>-12",
    "<-06>6",
    "<-09>9",
    "<+11>-11",
    "ChST-10",
    "HST10",
    "<+14>-14",
    "<+11>-11",
    "<+12>-12",
    "<+12>-12",
    "<-0930>9:30",
    "SST11",
    "<+12>-12",
    "<-11>11",
    "<+11>-11<+12>,M10.1.0,M4.1.0/3",
    "<+11>-11",
    "SST11",
    "<+09>-9",
    "<-08>8",
    "<+11>-11",
    "<+10>-10",
    "<-10>10",
    "ChST-10",
    "<-10>10",
    "<+12>-12",
    "<+13>-13",
    "<+12>-12",
    "<+12>-12",
    "UTC",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "<-03>3",
    "WGST3WGT,M3.5.6/22,M10.4.6/23", // OPENWRT: <-03>3<-02>,M3.5.0/-2,M10.5.0/-1 is error
    "<-03>3",
    "IST-5:30",     //Asia/Kolkata
    "<+0630>-6:30", // Asia/Yangon
    "<+0545>-5:45", //Asia/Kathmandu
    "LHST-10:30LHST-11,M10.1.0,M4.1.0",
    "EET-2",
    "HAST10HADT,M3.2.0,M11.1.0"
};

const char* timezone_names[] = {
    "Africa/Abidjan",
    "Africa/Accra",
    "Africa/Addis_Ababa",
    "Africa/Algiers",
    "Africa/Asmara",
    "Africa/Bamako",
    "Africa/Bangui",
    "Africa/Banjul",
    "Africa/Bissau",
    "Africa/Blantyre",
    "Africa/Brazzaville",
    "Africa/Bujumbura",
    "Africa/Cairo",
    "Africa/Casablanca",
    "Africa/Ceuta",
    "Africa/Conakry",
    "Africa/Dakar",
    "Africa/Dar_es_Salaam",
    "Africa/Djibouti",
    "Africa/Douala",
    "Africa/El_Aaiun",
    "Africa/Freetown",
    "Africa/Gaborone",
    "Africa/Harare",
    "Africa/Johannesburg",
    "Africa/Juba",
    "Africa/Kampala",
    "Africa/Khartoum",
    "Africa/Kigali",
    "Africa/Kinshasa",
    "Africa/Lagos",
    "Africa/Libreville",
    "Africa/Lome",
    "Africa/Luanda",
    "Africa/Lubumbashi",
    "Africa/Lusaka",
    "Africa/Malabo",
    "Africa/Maputo",
    "Africa/Maseru",
    "Africa/Mbabane",
    "Africa/Mogadishu",
    "Africa/Monrovia",
    "Africa/Nairobi",
    "Africa/Ndjamena",
    "Africa/Niamey",
    "Africa/Nouakchott",
    "Africa/Ouagadougou",
    "Africa/Porto-Novo",
    "Africa/Sao_Tome",
    "Africa/Tripoli",
    "Africa/Tunis",
    "Africa/Windhoek",
    "America/Adak",
    "America/Anchorage",
    "America/Anguilla",
    "America/Antigua",
    "America/Araguaina",
    "America/Argentina/Buenos_Aires",
    "America/Argentina/Catamarca",
    "America/Argentina/Cordoba",
    "America/Argentina/Jujuy",
    "America/Argentina/La_Rioja",
    "America/Argentina/Mendoza",
    "America/Argentina/Rio_Gallegos",
    "America/Argentina/Salta",
    "America/Argentina/San_Juan",
    "America/Argentina/San_Luis",
    "America/Argentina/Tucuman",
    "America/Argentina/Ushuaia",
    "America/Aruba",
    "America/Asuncion",
    "America/Atikokan",
    "America/Bahia",
    "America/Bahia_Banderas",
    "America/Barbados",
    "America/Belem",
    "America/Belize",
    "America/Blanc-Sablon",
    "America/Boa_Vista",
    "America/Bogota",
    "America/Boise",
    "America/Cambridge_Bay",
    "America/Campo_Grande",
    "America/Cancun",
    "America/Caracas",
    "America/Cayenne",
    "America/Cayman",
    "America/Chicago",
    "America/Chihuahua",
    "America/Costa_Rica",
    "America/Creston",
    "America/Cuiaba",
    "America/Curacao",
    "America/Danmarkshavn",
    "America/Dawson",
    "America/Dawson_Creek",
    "America/Denver",
    "America/Detroit",
    "America/Dominica",
    "America/Edmonton",
    "America/Eirunepe",
    "America/El_Salvador",
    "America/Fort_Nelson",
    "America/Fortaleza",
    "America/Glace_Bay",
    "America/Goose_Bay",
    "America/Grand_Turk",
    "America/Grenada",
    "America/Guadeloupe",
    "America/Guatemala",
    "America/Guayaquil",
    "America/Guyana",
    "America/Halifax",
    "America/Havana",
    "America/Hermosillo",
    "America/Indiana/Indianapolis",
    "America/Indiana/Knox",
    "America/Indiana/Marengo",
    "America/Indiana/Petersburg",
    "America/Indiana/Tell_City",
    "America/Indiana/Vevay",
    "America/Indiana/Vincennes",
    "America/Indiana/Winamac",
    "America/Inuvik",
    "America/Iqaluit",
    "America/Jamaica",
    "America/Juneau",
    "America/Kentucky/Louisville",
    "America/Kentucky/Monticello",
    "America/Kralendijk",
    "America/La_Paz",
    "America/Lima",
    "America/Los_Angeles",
    "America/Lower_Princes",
    "America/Maceio",
    "America/Managua",
    "America/Manaus",
    "America/Marigot",
    "America/Martinique",
    "America/Matamoros",
    "America/Mazatlan",
    "America/Menominee",
    "America/Merida",
    "America/Metlakatla",
    "America/Mexico_City",
    "America/Miquelon",
    "America/Moncton",
    "America/Monterrey",
    "America/Montevideo",
    "America/Montserrat",
    "America/Nassau",
    "America/New_York",
    "America/Nipigon",
    "America/Nome",
    "America/Noronha",
    "America/North_Dakota/Beulah",
    "America/North_Dakota/Center",
    "America/North_Dakota/New_Salem",
    "America/Nuuk",
    "America/Ojinaga",
    "America/Panama",
    "America/Pangnirtung",
    "America/Paramaribo",
    "America/Phoenix",
    "America/Port_of_Spain",
    "America/Port-au-Prince",
    "America/Porto_Velho",
    "America/Puerto_Rico",
    "America/Punta_Arenas",
    "America/Rainy_River",
    "America/Rankin_Inlet",
    "America/Recife",
    "America/Regina",
    "America/Resolute",
    "America/Rio_Branco",
    "America/Santarem",
    "America/Santiago",
    "America/Santo_Domingo",
    "America/Sao_Paulo",
    "America/Scoresbysund",
    "America/Sitka",
    "America/St_Barthelemy",
    "America/St_Johns",
    "America/St_Kitts",
    "America/St_Lucia",
    "America/St_Thomas",
    "America/St_Vincent",
    "America/Swift_Current",
    "America/Tegucigalpa",
    "America/Thule",
    "America/Thunder_Bay",
    "America/Tijuana",
    "America/Toronto",
    "America/Tortola",
    "America/Vancouver",
    "America/Whitehorse",
    "America/Winnipeg",
    "America/Yakutat",
    "America/Yellowknife",
    "Antarctica/Casey",
    "Antarctica/Davis",
    "Antarctica/DumontDUrville",
    "Antarctica/Macquarie",
    "Antarctica/Mawson",
    "Antarctica/McMurdo",
    "Antarctica/Palmer",
    "Antarctica/Rothera",
    "Antarctica/Syowa",
    "Antarctica/Troll",
    "Antarctica/Vostok",
    "Arctic/Longyearbyen",
    "Asia/Aden",
    "Asia/Almaty",
    "Asia/Amman",
    "Asia/Anadyr",
    "Asia/Aqtau",
    "Asia/Aqtobe",
    "Asia/Ashgabat",
    "Asia/Atyrau",
    "Asia/Baghdad",
    "Asia/Bahrain",
    "Asia/Baku",
    "Asia/Bangkok",
    "Asia/Barnaul",
    "Asia/Beirut",
    "Asia/Bishkek",
    "Asia/Brunei",
    "Asia/Chita",
    "Asia/Choibalsan",
    "Asia/Colombo",
    "Asia/Damascus",
    "Asia/Dhaka",
    "Asia/Dili",
    "Asia/Dubai",
    "Asia/Dushanbe",
    "Asia/Famagusta",
    "Asia/Gaza",
    "Asia/Hebron",
    "Asia/Ho_Chi_Minh",
    "Asia/Hong_Kong",
    "Asia/Hovd",
    "Asia/Irkutsk",
    "Asia/Jakarta",
    "Asia/Jayapura",
    "Asia/Jerusalem",
    "Asia/Kabul",
    "Asia/Kamchatka",
    "Asia/Karachi",
    "Asia/Kathmandu",
    "Asia/Khandyga",
    "Asia/Kolkata",
    "Asia/Krasnoyarsk",
    "Asia/Kuala_Lumpur",
    "Asia/Kuching",
    "Asia/Kuwait",
    "Asia/Macau",
    "Asia/Magadan",
    "Asia/Makassar",
    "Asia/Manila",
    "Asia/Muscat",
    "Asia/Nicosia",
    "Asia/Novokuznetsk",
    "Asia/Novosibirsk",
    "Asia/Omsk",
    "Asia/Oral",
    "Asia/Phnom_Penh",
    "Asia/Pontianak",
    "Asia/Pyongyang",
    "Asia/Qatar",
    "Asia/Qostanay",
    "Asia/Qyzylorda",
    "Asia/Riyadh",
    "Asia/Sakhalin",
    "Asia/Samarkand",
    "Asia/Seoul",
    "Asia/Shanghai",
    "Asia/Singapore",
    "Asia/Srednekolymsk",
    "Asia/Taipei",
    "Asia/Tashkent",
    "Asia/Tbilisi",
    "Asia/Tehran",
    "Asia/Thimphu",
    "Asia/Tokyo",
    "Asia/Tomsk",
    "Asia/Ulaanbaatar",
    "Asia/Urumqi",
    "Asia/Ust-Nera",
    "Asia/Vientiane",
    "Asia/Vladivostok",
    "Asia/Yakutsk",
    "Asia/Yangon",
    "Asia/Yekaterinburg",
    "Asia/Yerevan",
    "Atlantic/Azores",
    "Atlantic/Bermuda",
    "Atlantic/Canary",
    "Atlantic/Cape_Verde",
    "Atlantic/Faroe",
    "Atlantic/Madeira",
    "Atlantic/Reykjavik",
    "Atlantic/South_Georgia",
    "Atlantic/St_Helena",
    "Atlantic/Stanley",
    "Australia/Adelaide",
    "Australia/Brisbane",
    "Australia/Broken_Hill",
    "Australia/Currie",
    "Australia/Darwin",
    "Australia/Eucla",
    "Australia/Hobart",
    "Australia/Lindeman",
    "Australia/Lord_Howe",
    "Australia/Melbourne",
    "Australia/Perth",
    "Australia/Sydney",
    "Etc/GMT",
    "Etc/GMT+1",
    "Etc/GMT+10",
    "Etc/GMT+11",
    "Etc/GMT+12",
    "Etc/GMT+2",
    "Etc/GMT+3",
    "Etc/GMT+4",
    "Etc/GMT+5",
    "Etc/GMT+6",
    "Etc/GMT+7",
    "Etc/GMT+8",
    "Etc/GMT+9",
    "Etc/GMT-1",
    "Etc/GMT-10",
    "Etc/GMT-11",
    "Etc/GMT-12",
    "Etc/GMT-13",
    "Etc/GMT-14",
    "Etc/GMT-2",
    "Etc/GMT-3",
    "Etc/GMT-4",
    "Etc/GMT-5",
    "Etc/GMT-6",
    "Etc/GMT-7",
    "Etc/GMT-8",
    "Etc/GMT-9",
    "Europe/Amsterdam",
    "Europe/Andorra",
    "Europe/Astrakhan",
    "Europe/Athens",
    "Europe/Belgrade",
    "Europe/Berlin",
    "Europe/Bratislava",
    "Europe/Brussels",
    "Europe/Bucharest",
    "Europe/Budapest",
    "Europe/Busingen",
    "Europe/Chisinau",
    "Europe/Copenhagen",
    "Europe/Dublin",
    "Europe/Gibraltar",
    "Europe/Guernsey",
    "Europe/Helsinki",
    "Europe/Isle_of_Man",
    "Europe/Istanbul",
    "Europe/Jersey",
    "Europe/Kaliningrad",
    "Europe/Kiev",
    "Europe/Kirov",
    "Europe/Lisbon",
    "Europe/Ljubljana",
    "Europe/London",
    "Europe/Luxembourg",
    "Europe/Madrid",
    "Europe/Malta",
    "Europe/Mariehamn",
    "Europe/Minsk",
    "Europe/Monaco",
    "Europe/Moscow",
    "Europe/Oslo",
    "Europe/Paris",
    "Europe/Podgorica",
    "Europe/Prague",
    "Europe/Riga",
    "Europe/Rome",
    "Europe/Samara",
    "Europe/San_Marino",
    "Europe/Sarajevo",
    "Europe/Saratov",
    "Europe/Simferopol",
    "Europe/Skopje",
    "Europe/Sofia",
    "Europe/Stockholm",
    "Europe/Tallinn",
    "Europe/Tirane",
    "Europe/Ulyanovsk",
    "Europe/Uzhgorod",
    "Europe/Vaduz",
    "Europe/Vatican",
    "Europe/Vienna",
    "Europe/Vilnius",
    "Europe/Volgograd",
    "Europe/Warsaw",
    "Europe/Zagreb",
    "Europe/Zaporozhye",
    "Europe/Zurich",
    "Indian/Antananarivo",
    "Indian/Chagos",
    "Indian/Christmas",
    "Indian/Cocos",
    "Indian/Comoro",
    "Indian/Kerguelen",
    "Indian/Mahe",
    "Indian/Maldives",
    "Indian/Mauritius",
    "Indian/Mayotte",
    "Indian/Reunion",
    "Pacific/Apia",
    "Pacific/Auckland",
    "Pacific/Bougainville",
    "Pacific/Chatham",
    "Pacific/Chuuk",
    "Pacific/Easter",
    "Pacific/Efate",
    "Pacific/Enderbury",
    "Pacific/Fakaofo",
    "Pacific/Fiji",
    "Pacific/Funafuti",
    "Pacific/Galapagos",
    "Pacific/Gambier",
    "Pacific/Guadalcanal",
    "Pacific/Guam",
    "Pacific/Honolulu",
    "Pacific/Kiritimati",
    "Pacific/Kosrae",
    "Pacific/Kwajalein",
    "Pacific/Majuro",
    "Pacific/Marquesas",
    "Pacific/Midway",
    "Pacific/Nauru",
    "Pacific/Niue",
    "Pacific/Norfolk",
    "Pacific/Noumea",
    "Pacific/Pago_Pago",
    "Pacific/Palau",
    "Pacific/Pitcairn",
    "Pacific/Pohnpei",
    "Pacific/Port_Moresby",
    "Pacific/Rarotonga",
    "Pacific/Saipan",
    "Pacific/Tahiti",
    "Pacific/Tarawa",
    "Pacific/Tongatapu",
    "Pacific/Wake",
    "Pacific/Wallis",
    "UTC",
    "America/Buenos_Aires",
    "America/Catamarca",
    "America/Cordoba",
    "America/Jujuy",
    "America/La_Rioja",
    "America/Mendoza",
    "America/Rio_Gallegos",
    "America/Salta",
    "America/San_Juan",
    "America/San_Luis",
    "America/Tucuman",
    "America/Ushuaia",
    "America/Godthab",
    "America/Santa_Isabel",
    "Asia/Calcutta",
    "Asia/Rangoon",
    "Asia/Katmandu",
    "Australia/LHI",
    "Libya",
    "US/Aleutian"
};

const int timezone_num = sizeof( timezone_names )/ sizeof( char* );

const struct country_list countries[] =
{
  { "008", "Albania" },
  { "012", "Algeria" },
  { "016", "American Samoa" },
  { "032", "Argentina" },
  { "051", "Armenia" },
  { "533", "Aruba" },
  { "036", "Australia" },
  { "040", "Austria" },
  { "031", "Azerbaijan" },
  { "048", "Bahrain" },
  { "050", "Bangladesh" },
  { "052", "Barbados" },
  { "112", "Belarus" },
  { "056", "Belgium" },
  { "084", "Belize" },
  { "068", "Bolivia" },
  { "070", "Bosnia and Herzegovina" },
  { "076", "Brazil" },
  { "096", "Brunei Darussalam" },
  { "100", "Bulgaria" },
  { "116", "Cambodia" },
  { "124", "Canada" },
  { "152", "Chile" },
  { "156", "China" },
  { "170", "Colombia" },
  { "188", "Costa Rica" },
  { "191", "Croatia" },
  { "196", "Cyprus" },
  { "203", "Czech Republic" },
  { "208", "Denmark" },
  { "214", "Dominican Republic" },
  { "218", "Ecuador" },
  { "818", "Egypt" },
  { "222", "El Salvador" },
  { "233", "Estonia" },
  { "246", "Finland" },
  { "250", "France" },
  { "268", "Georgia" },
  { "276", "Germany" },
  { "300", "Greece" },
  { "304", "Greenland" },
  { "308", "Grenada" },
  { "316", "Guam" },
  { "320", "Guatemala" },
  { "332", "Haiti" },
  { "340", "Honduras" },
  { "344", "Hong Kong" },
  { "348", "Hungary" },
  { "352", "Iceland" },
  { "356", "India" },
  { "360", "Indonesia" },
  { "372", "Ireland" },
  { "376", "Israel" },
  { "380", "Italy" },
  { "388", "Jamaica" },
  { "392", "Japan" },
  { "400", "Jordan" },
  { "398", "Kazakhstan" },
  { "404", "Kenya" },
  { "414", "Kuwait" },
  { "428", "Latvia" },
  { "422", "Lebanon" },
  { "438", "Liechtenstein" },
  { "440", "Lithuania" },
  { "442", "Luxembourg" },
  { "446", "Macau" },
  { "807", "Macedonia" },
  { "458", "Malaysia" },
  { "470", "Malta" },
  { "484", "Mexico" },
  { "492", "Monaco" },
  { "499", "Montenegro" },
  { "504", "Morocco" },
  { "524", "Nepal" },
  { "528", "Netherlands" },
  { "530", "Netherlands Antilles" },
  { "554", "New Zealand" },
  { "580", "Northern Mariana Islands" },
  { "578", "Norway" },
  { "512", "Oman" },
  { "586", "Pakistan" },
  { "591", "Panama" },
  { "598", "Papua New Guinea" },
  { "604", "Peru" },
  { "608", "Philippines" },
  { "616", "Poland" },
  { "620", "Portugal" },
  { "630", "Puerto Rico" },
  { "634", "Qatar" },
  { "688", "Republic of Serbia" },
  { "642", "Romania" },
  { "643", "Russia" },
  { "646", "Rwanda" },
  { "682", "Saudi Arabia" },
  { "702", "Singapore" },
  { "703", "Slovakia" },
  { "705", "Slovenia" },
  { "710", "South Africa" },
  { "410", "South Korea" },
  { "724", "Spain" },
  { "144", "Sri Lanka" },
  { "752", "Sweden" },
  { "756", "Switzerland" },
  { "158", "Taiwan" },
  { "764", "Thailand" },
  { "780", "Trinidad and Tobago" },
  { "788", "Tunisia" },
  { "792", "Turkey" },
  { "800", "Uganda" },
  { "804", "Ukraine" },
  { "784", "United Arab Emirates" },
  { "826", "United Kingdom" },
  { "840", "United States" },
  { "858", "Uruguay" },
  { "860", "Uzbekistan" },
  { "862", "Venezuela" },
  { "704", "Vietnam" },
  { "850", "Virgin Islands, U.S." },
  { "887", "Yemen" },
  { "716", "Zimbabwe" }
};

const int countries_num = sizeof( countries )/sizeof( countries[0] );


//=================
//  Locals
//=================
/*
 * Private Date
 */
static struct blob_buf buf;
static const struct blobmsg_policy system_policy[__SYSTEM_SET_MAX] = {
    [SYSTEM_HOSTNAME] = { .name = "hostname", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_LOG_FILE] = { .name = "log_file", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_LOG_SIZE] = { .name = "log_size", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_LOGLEVEL] = { .name = "log_level", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_TIMEZONE] = { .name = "timezone", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_LOG_URI]  = { .name = "log_ip", .type = BLOBMSG_TYPE_STRING },
    [SYSTEM_LOG_PRO]  = { .name = "log_proto", .type = BLOBMSG_TYPE_STRING },

};

static const struct uci_blob_param_list system_list = {
    .n_params = __SYSTEM_SET_MAX,
    .params = system_policy,
};

//=================
//  Functions
//=================
/*
 * Private Functions
 */
static int
cfparse_sys_basic_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
);

//=============================================================================
static void
cfparse_sys_node_free(
    struct sys_config_parse *cfpar
)
//=============================================================================
{
    SAFE_FREE( cfpar->cf_section.config );
    SAFE_FREE( cfpar );
}

//=============================================================================
static void
cfparse_sys_tree_free(
    struct sys_config_parse *cfpar
)
//=============================================================================
{
    avl_delete( &basic_system_vltree.avl, &cfpar->node.avl );
    cfparse_sys_node_free( cfpar );
}


//=============================================================================
void
cfparse_sys_update_cfg(
    struct sys_config_parse *cfpar_old,
    struct sys_config_parse *cfpar_new
)
//=============================================================================
{
    struct blob_attr *tb_old[__SYSTEM_SET_MAX];
    struct blob_attr *tb_new[__SYSTEM_SET_MAX];
    int i = 0;
    int option = 0;

    struct blob_attr *config_old = cfpar_old->cf_section.config;
    struct blob_attr *config_new = cfpar_new->cf_section.config;

    blobmsg_parse( system_policy,
            __SYSTEM_SET_MAX,
            tb_old,
            blob_data( config_old ),
            blob_len( config_old ) );

    blobmsg_parse( system_policy,
            __SYSTEM_SET_MAX,
            tb_new,
            blob_data( config_new ),
            blob_len( config_new ) );

    for( i = 0; i < __SYSTEM_SET_MAX; i++ ) {
        if( !blob_attr_equal( tb_new[i], tb_old[i] ) ) {
            option |= cfparse_sys_basic_hooker( tb_new, i, NULL );
        }
    }

    if ( option ) {
        if ( option & OPTION_FLAGS_NEED_RELOAD ) {
            apply_set_reload_flag( CONFIG_TRACK_SYSTEM );
        }
    }

}

//=============================================================================
static void
cfparse_sys_vltree_update(
    struct vlist_tree *tree,
    struct vlist_node *node_new,
    struct vlist_node *node_old
)
//=============================================================================
{
    struct sys_config_parse *cfpar_old = NULL;
    struct sys_config_parse *cfpar_new = NULL;

    if ( node_old ) {
        cfpar_old =
            container_of(node_old, struct sys_config_parse, node);
    }

    if ( node_new ) {
        cfpar_new =
            container_of(node_new, struct sys_config_parse, node);
    }

    if ( cfpar_old && cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "update system section '%s'\n", cfpar_old->cf_section.name );

        if ( blob_attr_equal( cfpar_new->cf_section.config, cfpar_old->cf_section.config ) ) {
            cfparse_sys_node_free( cfpar_new );

            return;
        }

        cfparse_sys_update_cfg( cfpar_old, cfpar_new );

        SAFE_FREE( cfpar_old->cf_section.config );
        cfpar_old->cf_section.config = blob_memdup( cfpar_new->cf_section.config );
        cfparse_sys_node_free( cfpar_new );
    }
    else if ( cfpar_old ) {
        cfmanager_log_message( L_WARNING,
            "Delete system section '%s'\n", cfpar_old->cf_section.name );

        cfparse_sys_tree_free( cfpar_old );

        apply_set_reload_flag( CONFIG_TRACK_SYSTEM );

    }
    else if ( cfpar_new ) {
        cfmanager_log_message( L_WARNING,
            "New network section '%s'\n", cfpar_new->cf_section.name );

        apply_set_reload_flag( CONFIG_TRACK_SYSTEM );

    }
}

//=============================================================================
static int
cfparse_sys_basic_hooker(
    struct blob_attr **new_config,
    int index,
    struct cm_vltree_extend *extend
)
//=============================================================================
{
    int rc = 0;

    switch( index ) {
        case SYSTEM_TIMEZONE:
            rc |= OPTION_FLAGS_NEED_RELOAD;
            break;
        case SYSTEM_LOGLEVEL:
            rc |= OPTION_FLAGS_NEED_RELOAD;
            break;
        case SYSTEM_LOG_PRO:
            rc |= OPTION_FLAGS_NEED_RELOAD;
            break;
        case SYSTEM_LOG_URI:
            rc |= OPTION_FLAGS_NEED_RELOAD;
            break;

        default:
            break;
    }

    return rc;
}

//=============================================================================
static void
cfparse_sys_add_blob_to_tree(
    struct blob_attr *data,
    const char *section_name,
    const char *section_type
)
//=============================================================================
{
    struct sys_config_parse *cfpar = NULL;
    char *name;
    char *type;

    cfpar = calloc_a( sizeof( *cfpar ),
        &name, strlen( section_name ) +1,
        &type, strlen( section_type ) +1 );

    if ( !cfpar ) {
        cfmanager_log_message( L_ERR,
            "failed to malloc network_config_parse '%s'\n", section_name );

        return;
    }

    cfpar->cf_section.name = strcpy( name, section_name );
    cfpar->cf_section.type = strcpy( type, section_type );
    cfpar->cf_section.config = blob_memdup( data );

    vlist_add( &basic_system_vltree, &cfpar->node, cfpar->cf_section.name );
}


//=============================================================================
static void
cfparse_sys_uci_to_blob(
    struct uci_section *s
)
//=============================================================================
{
    blob_buf_init( &buf, 0 );
    uci_to_blob( &buf, s, &system_list );
    cfparse_sys_add_blob_to_tree( buf.head, s->e.name, s->type );
    blob_buf_free( &buf );
}

//=============================================================================
int
cfparse_load_sys(
    void
)
//=============================================================================
{
    struct uci_element *e = NULL;
    struct uci_package *package = NULL;

    check_set_defvalue( CHECK_SYSTEM );

    package = cfparse_init_package( "system" );
    if ( !package ) {
        cfmanager_log_message( L_ERR, "load system package failed\n" );
        return -1;
    }

    vlist_update( &basic_system_vltree );

    uci_foreach_element( &package->sections, e ) {
        struct uci_section *s = uci_to_section( e );
        if( 0 == strcmp( s->type, "system" ) ) {
            cfparse_sys_uci_to_blob( s );
        }
    }

    vlist_flush( &basic_system_vltree );

    if( apply_get_reload_flag( CONFIG_TRACK_SYSTEM ) ) {
        apply_add( "system" );
        apply_flush_reload_flag( CONFIG_TRACK_SYSTEM );
        apply_timer_start();
    }

    return 0;
}

//=============================================================================
void
cfparse_sys_init(
     void
)
//=============================================================================
{
    vlist_init( &basic_system_vltree, avl_strcmp, cfparse_sys_vltree_update );
    basic_system_vltree.keep_old = true;
    basic_system_vltree.no_delete = true;
}

//=============================================================================
void
cfparse_sys_deinit(
     void
)
//=============================================================================
{
    vlist_flush_all( &basic_system_vltree );
}
