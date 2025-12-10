#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <algorithm>
#include "student.h"

void vector_example()
{
    std::cout << "\n=== Vector Example ===" << std::endl;

    // 创建和初始化vector
    std::vector<int> numbers = {1, 2, 3, 4, 5};

    // 添加元素
    numbers.push_back(6);
    numbers.insert(numbers.begin(), 0);

    // 遍历vector
    std::cout << "Vector elements: ";
    for (const auto &num : numbers)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // 使用算法
    auto it = std::find(numbers.begin(), numbers.end(), 3);
    if (it != numbers.end())
    {
        std::cout << "Found 3 at position: " << std::distance(numbers.begin(), it) << std::endl;
    }

    std::cout << "Vector size: " << numbers.size() << std::endl;
    std::cout << "Vector capacity: " << numbers.capacity() << std::endl;
}

void list_example()
{
    std::cout << "\n=== List Example ===" << std::endl;

    std::list<std::string> names = {"Alice", "Bob", "Charlie"};

    // 在列表中间插入元素
    auto it = names.begin();
    std::advance(it, 1); // 移动到第二个位置
    names.insert(it, "David");

    // 遍历列表
    std::cout << "List elements: ";
    for (const auto &name : names)
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    // 删除元素
    names.remove("Bob");
    std::cout << "After removing Bob: ";
    for (const auto &name : names)
    {
        std::cout << name << " ";
    }
    std::cout << std::endl;
}

void map_example()
{
    std::cout << "\n=== Map Example ===" << std::endl;

    std::map<std::string, int> age_map = {
        {"Alice", 25},
        {"Bob", 30},
        {"Charlie", 35}};

    // 添加新元素
    age_map["David"] = 28;
    age_map.insert({"Eve", 32});

    // 遍历map
    std::cout << "Age map:" << std::endl;
    for (const auto &pair : age_map)
    {
        std::cout << pair.first << ": " << pair.second << " years old" << std::endl;
    }

    // 查找元素
    auto search = age_map.find("Alice");
    if (search != age_map.end())
    {
        std::cout << "Found Alice, age: " << search->second << std::endl;
    }

    // 检查元素是否存在
    if (age_map.count("Frank") == 0)
    {
        std::cout << "Frank not found in the map" << std::endl;
    }
}

void set_example()
{
    std::cout << "\n=== Set Example ===" << std::endl;

    std::set<int> unique_numbers = {1, 2, 3, 4, 5, 5, 4, 3}; // 重复元素会被自动去重

    // 添加元素
    unique_numbers.insert(6);
    unique_numbers.insert(2); // 不会添加重复元素

    // 遍历set
    std::cout << "Unique numbers: ";
    for (const auto &num : unique_numbers)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    // 检查元素是否存在
    if (unique_numbers.find(3) != unique_numbers.end())
    {
        std::cout << "3 exists in the set" << std::endl;
    }

    // 删除元素
    unique_numbers.erase(4);
    std::cout << "After removing 4: ";
    for (const auto &num : unique_numbers)
    {
        std::cout << num << " ";
    }
    std::cout << std::endl;
}

void unordered_collections_example()
{
    std::cout << "\n=== Unordered Collections Example ===" << std::endl;

    // unordered_map 示例
    std::unordered_map<std::string, std::string> capital_cities = {
        {"China", "Beijing"},
        {"USA", "Washington D.C."},
        {"Japan", "Tokyo"},
        {"UK", "London"}};

    std::cout << "Capital cities:" << std::endl;
    for (const auto &pair : capital_cities)
    {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    // unordered_set 示例
    std::unordered_set<std::string> programming_languages = {"C++", "Python", "Java", "JavaScript", "C++"};

    std::cout << "Programming languages (unordered set): ";
    for (const auto &lang : programming_languages)
    {
        std::cout << lang << " ";
    }
    std::cout << std::endl;
}

void student_collections_example()
{
    std::cout << "\n=== Student Collections Example ===" << std::endl;

    // 创建学生对象
    Student student1("张三", 18, "高一(1)班");
    Student student2("李四", 17, "高一(2)班");
    Student student3("王五", 19, "高一(1)班");
    Student student4("赵六", 18, "高一(3)班");
    Student student5("钱七", 17, "高一(2)班");

    // 1. 使用vector存储学生列表
    std::cout << "\n1. Vector of Students:" << std::endl;
    std::vector<Student> students = {student1, student2, student3, student4, student5};
    for (const auto &student : students)
    {
        std::cout << "  " << student << std::endl;
    }

    // 2. 使用map按班级分组学生
    std::cout << "\n2. Map of Students by Class:" << std::endl;
    std::map<std::string, std::vector<Student>> classMap;
    for (const auto &student : students)
    {
        classMap[student.getClassName()].push_back(student);
    }

    for (const auto &pair : classMap)
    {
        std::cout << "班级: " << pair.first << std::endl;
        for (const auto &student : pair.second)
        {
            std::cout << "  - " << student.getName() << " (年龄: " << student.getAge() << ")" << std::endl;
        }
    }

    // 3. 使用set存储学生（按姓名排序）
    std::cout << "\n3. Set of Students (sorted by name):" << std::endl;
    std::set<Student> studentSet = {student1, student2, student3, student4, student5};
    for (const auto &student : studentSet)
    {
        std::cout << "  " << student << std::endl;
    }

    // 4. 查找特定学生
    std::cout << "\n4. Searching for students:" << std::endl;
    auto it = std::find_if(students.begin(), students.end(),
                           [](const Student &s)
                           { return s.getName() == "李四"; });
    if (it != students.end())
    {
        std::cout << "找到学生: " << *it << std::endl;
    }

    // 5. 统计各年龄段学生数量
    std::cout << "\n5. Student age statistics:" << std::endl;
    std::map<int, int> ageCount;
    for (const auto &student : students)
    {
        ageCount[student.getAge()]++;
    }
    for (const auto &pair : ageCount)
    {
        std::cout << "年龄 " << pair.first << " 岁: " << pair.second << " 人" << std::endl;
    }
}
