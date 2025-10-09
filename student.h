#ifndef STUDENT_H
#define STUDENT_H

#include <string>
#include <iostream>

class Student
{
private:
    std::string name;
    int age;
    std::string className;

public:
    // 构造函数
    Student(const std::string &name = "", int age = 0, const std::string &className = "")
        : name(name), age(age), className(className) {}

    // Getter 方法
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getClassName() const { return className; }

    // Setter 方法
    void setName(const std::string &newName) { name = newName; }
    void setAge(int newAge) { age = newAge; }
    void setClassName(const std::string &newClassName) { className = newClassName; }

    // 显示学生信息
    void display() const
    {
        std::cout << "姓名: " << name << ", 年龄: " << age << ", 班级: " << className;
    }

    // 重载输出运算符
    friend std::ostream &operator<<(std::ostream &os, const Student &student)
    {
        os << "姓名: " << student.name << ", 年龄: " << student.age << ", 班级: " << student.className;
        return os;
    }

    // 重载比较运算符（用于set和map排序）
    bool operator<(const Student &other) const
    {
        return name < other.name;
    }

    // 重载相等运算符
    bool operator==(const Student &other) const
    {
        return name == other.name && age == other.age && className == other.className;
    }
};

#endif // STUDENT_H
