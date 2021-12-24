/*
 * @lc app=leetcode.cn id=912 lang=cpp
 *
 * [912] 排序数组
 */

// @lc code=start
class Solution {
public:
    vector<int> sortArray(vector<int>& nums) {
        quick_sort(nums, 0, nums.size() - 1);
        return nums;
    }
    void quick_sort(vector<int> &nums, int start, int end)
    {
        if (start >= end)
        {
            return ;
        }
        int left = start;
        int right = end;
        int key = rand() % (right - left + 1) + left;
        
        swap(nums[start], nums[key]);
        int provie = nums[start];

        while (left < right)
        {
            while (left < right && nums[right] >= provie) 
            {
                right--;
            }
            nums[left] = nums[right];
            while (left < right && nums[left] <= provie) 
            {
                left++;
            }
            nums[right] = nums[left];
        }
        nums[left] = provie;
        quick_sort(nums, start, left - 1);
        quick_sort(nums, left + 1, end);
    }
};
// @lc code=end

