/*
 * @lc app=leetcode.cn id=904 lang=cpp
 *
 * [904] 水果成篮
 */

// @lc code=start
class Solution {
public:
    int totalFruit(vector<int>& fruits) {
        int left = 0;
        int right = height.size() - 1;
        int result = 0;
        while (left < right)
        {
            int h = min(height[left], height[right]);
            int key = h * (right - left);
            result = max(result, key);
            if (height[left] < height[right]) 
            {
                ++left;
            }          
            else 
            {
                --right;
            } 
        }
        return result;
    }
};
// @lc code=end

