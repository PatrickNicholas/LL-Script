# 介绍

simple but powerful script, and I am rewriting it.

## Syntax

[EBNF](https://github.com/thinkermao/LL-Script/blob/master/grammar.md) descriped syntax of this script.

## Updata:
	
2015/04/29：
修改了部分命名规则；
将部分臃肿代码精简化；
修改了变量是否定义的验证方式，使得效率提升近5倍。
    
2015/5/1：
对部分内容进行了重构，简化代码结构。
	
2015/12/29：
准备重构工作，设计如下：

- 基础数据类型 integer, float, character, string, array([]), function
- 支持基本语句 if, while, return, break, continue
- 尾递归优化，异常处理，柯里化函数
- VM, GC

2015/12/30：
重写 lexer 和 Parser, 添加单元测试。

2016/1/1：
重新设计抽象语法树部分。

2016/1/2：
完成语法制导翻译部分。

2016/1/14：
使用智能指针代替手工释放内存

2016/1/15：
修改 grammar 以支持 lambda
添加 pretty print 功能，打印抽象语法树

2016/1/20：
将 AST 转换成 CFG，完成部分死代码删除
