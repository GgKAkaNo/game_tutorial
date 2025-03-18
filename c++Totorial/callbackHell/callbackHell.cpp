#include <thread>
#include <fstream>
#include <string>
#include <chrono>
#include <iostream>
#include <functional>

using namespace std;

// 异步读取文件
void async_read_file(const string& filename,
                    function<void(const string&)> callback)
{
    thread t([filename, callback]()
    {
        this_thread::sleep_for(chrono::seconds(1));

        ifstream file(filename);
        string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        callback(content);
    });
    t.detach();
}

void async_write_file(const string& filename, const string& content,
                      function<void()> callback)
{
    thread t([filename, content, callback]()
    {
        this_thread::sleep_for(chrono::seconds(1));

        ofstream ofs(filename);
        ofs << content;
        callback();
    });
    t.detach();
}

// 读取文件 -> 处理数据 -> 写入文件
void process_file(const string &input_filename, const string &output_filename)
{
    async_read_file(input_filename, [output_filename](const string &content)
    {
        // 处理数据
        string processed_content = content + "processed";

        async_write_file(output_filename, processed_content,
                     []() { cout << "File written successfully!" << endl; });
    });
}

int main() {
  process_file("input.txt", "output.txt");

  // 主线程继续执行其他任务
  for (int i = 0; i < 5; i++) {
    cout << "Main thread is working..." << endl;
    this_thread::sleep_for(chrono::seconds(1));
  }

  return 0;
}