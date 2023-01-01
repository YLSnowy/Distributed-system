#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <sys/time.h>
using namespace std;

vector<vector<string>> readfile(int n, int m)
{
	vector<vector<string>> vec;
	int i = 0, j = 0;
	ifstream file;
	file.open("input.txt");
	string str;

	while (file >> str)
	{
		vec.push_back(vector<string>());
		vec[i].push_back(str);
		j++;
		if (j == m) //输入文件的列数超过了规定的列数
		{
			j = 0;
			i++;
		}
		if (i == n) // 输入文件的行数超过了规定的行数
		{
			vec.resize(n);
			break;
		}
	}

	file.close();
	return vec;
}

// 输出文件内容，也就是事件矩阵
void displayString(vector<vector<string>> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		for (int j = 0; j < vec[i].size(); j++)
		{
			cout << vec[i][j] << " ";
		}
		cout << endl;
	}
}

// 输出结果矩阵，也就是逻辑时钟
void PrintMatrix(vector<vector<int>> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		for (int j = 0; j < vec[i].size(); j++)
		{
			cout << vec[i][j] << " ";
		}
		cout << endl;
	}
}


int main()
{
	int n, m;
	int s = 0, r = 0;
	bool finished = false;
	string senders[9] = {"s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9"};
	string receivers[9] = {"r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9"};
	vector<vector<int>> matrix;
	cout << "Please input the rows of matrix" << endl;
	cin >> n;
	cout << "Please input the cols of matrix" << endl;
	cin >> m;
	
	struct timespec start_time, end_time;
    clock_gettime(CLOCK_REALTIME, &start_time);

	int count[n];
	for (int i = 0; i < n; i++)
	{
		count[i] = 0;
	}
	vector<vector<string>> vec = readfile(n, m);

	for (int i = 0; i < vec.size(); i++)
	{
		matrix.push_back(vector<int>());
		for (int j = 0; j < vec[i].size(); j++)
		{
			matrix[i].push_back(0);
		}
	}
	while (finished == false)
	{
		for (int i = 0; i < vec.size(); i++)
		{
			for (int j = 0; j < vec[i].size(); j++)
			{
				if (vec[i][j].length() == 1 && count[i] == j)
				{
					if (j == 0)
					{
						matrix[i][j] = 1;
						count[i]++;
					}
					else
					{
						matrix[i][j] = matrix[i][j - 1] + 1;
						count[i] += 1;
					}
				}
				if (vec[i][j].find("s") == 0 && count[i] == j)
				{
					if (j == 0)
					{
						matrix[i][j] = 1;
						count[i] += 1;
					}
					else
					{
						matrix[i][j] = matrix[i][j - 1] + 1;
						count[i] += 1;
					}
				}
				if (vec[i][j].find("r") == 0 && count[i] == j)
				{
					if (vec[i][j].compare(receivers[r]) == 0)
					{ 
						for (int k = 0; k < vec.size(); k++)
						{
							for (int l = 0; l < vec[k].size(); l++)
							{
								if (vec[k][l].compare(senders[s]) == 0)
								{
									if (j == 0)
									{
										matrix[i][j] = max(0, matrix[k][l]) + 1;
										count[i] += 1;
									}
									else
									{
										matrix[i][j] = max(matrix[i][j - 1], matrix[k][l]) + 1;
										count[i] += 1;
									}
								}
							}
						}
						s++;
						r++;
					}
				}
				if (vec[i][j].length() == 4 && count[i] == j)
				{
					count[i] += 1;
				}
			}
		}
		finished = true;
		for (int i = 0; i < n; i++)
		{
			if (count[i] != vec[i].size())
			{
				finished = false;
			}
		}
	}

	cout << "Message Matrix: " << endl;
	displayString(vec);
	cout << "Logical Time Matrix: " << endl;
	PrintMatrix(matrix);
    
	clock_gettime(CLOCK_REALTIME, &end_time);
    cout << "The Time to complete the algorithm is " << end_time.tv_sec - start_time.tv_sec << "s " << end_time.tv_nsec - start_time.tv_nsec << "ns" << endl;
	return 0;
}