<!--
 * @Author: Corvo Attano(fkxzz001@qq.com)
 * @Description: 
 * @LastEditors: Corvo Attano(fkxzz001@qq.com)
-->
# 多线程协调词频统计
* Usage: word_count [dirname]
## 主线程
* 遍历目标文件夹及子文件夹，找到.txt文件，保存其路径和大小
* 将文件按大小平均分配到子线程中进行统计
* 子线程
## 子线程
* 当分割点在单词内的时候，会导致一个词被分成两个词，为防止这种情况出现，每个工作线程处理文本时，若不是从文件偏移0处开始处理，则，放弃第一个词，并多处理后一个词