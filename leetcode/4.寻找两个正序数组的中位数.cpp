/*
 * @lc app=leetcode.cn id=4 lang=cpp
 *
 * [4] 寻找两个正序数组的中位数
 */
#include <vector>
// @lc code=start
class Solution {
public:
    double findMedianSortedArrays(vector<int>& nums1, vector<int>& nums2) {
        int n = nums1.size();
        int m = nums2.size();
        int len = n + m;
        if (len % 2) 
        {
            return fun(nums1, 0, n, nums2, 0, m, (len + 1) / 2);
        }
        else 
        {
            double x = fun(nums1, 0, n, nums2, 0, m, (len + 1) / 2);
            cout << "x = " << x <<endl;
            double y = fun(nums1, 0, n, nums2, 0, m, (len + 1) / 2 + 1);
            cout << "y = " << y <<endl;
            return (x + y) / 2;
        }
    }

    double fun(vector<int> &nums1, int n1_start, int n1_end, vector<int> &nums2, int n2_start, int n2_end, int k)
    {
        if (n1_start >=  n1_end) 
        {
            return nums2[n2_start + k - 1];
        }
        if (n2_start >= n2_end) 
        {
            return nums1[n1_start + k - 1];
        }
        if (k == 1)
        {
            return min(nums1[n1_start], nums2[n2_start]);
        }
        int mid = k / 2;
        double x = n1_start + mid - 1>= n1_end ? INT_MAX : nums1[n1_start + mid - 1];
        double y = n2_start + mid - 1>= n2_end ? INT_MAX : nums2[n2_start + mid - 1];
        if (x < y) 
        {
            return fun(nums1, n1_start + mid, n1_end, nums2, n2_start, n2_end, k - mid);
        }
        else 
        {
            return fun(nums1, n1_start, n1_end, nums2, n2_start + mid, n2_end, k - mid);
        }
    }
};
// @lc code=end

