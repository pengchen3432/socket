/*
 * @lc app=leetcode.cn id=11 lang=cpp
 *
 * [11] 盛最多水的容器
 */

// @lc code=start
class Solution {
public:
    int maxArea(vector<int>& height) {
        int left = 0;
        int right = height.size() - 1;
        int result = 0;
        int area = 0;
        while (left < right) 
        {
            if (height[left] < height[right])
            {
                area = height[left] * (right - left);
                left++;
            }
            else 
            {
                area = height[right] * (right - left);
                right--;
            }
            result = max(area, result);
        }
        return result;
    }
};
// @lc code=end

