-- Two dashes start a one-line comment.注释

--[[
     Adding two ['s and ]'s makes it a
     multi-line comment.
     多行注释
--]]

----------------------------------------------------
-- 1. Variables and flow control.
-- 1. 变量和流程控制
----------------------------------------------------

num = 42  -- Numbers can be integer or floating point. 数字可以是整数或浮点数

s = 'walternate'  -- Immutable strings like Python. 不可变字符串
t = "double-quotes are also fine" -- Strings can use single or double quotes. 字符串可以使用单引号或双引号
u = [[ Double brackets 
       start and end
       multi-line strings.]] -- 两个方括号开始和结束多行字符串
t = nil  -- Undefines t; Lua has garbage collection. 未定义t；Lua有垃圾回收

-- Blocks are denoted with keywords like do/end: 代码块用do/end关键字表示
while num < 50 do
  num = num + 1  -- No ++ or += type operators. 没有++或+=类型的运算符
end

-- If clauses: if语句
if num > 40 then
  print('over 40')
elseif s ~= 'walternate' then  -- ~= is not equals. ~=不等于
  -- Equality check is == like Python; ok for strs. 等于检查是==像Python；对于字符串是ok的
  io.write('not over 40\n')  -- Defaults to stdout. 默认为标准输出
else
  -- Variables are global by default. 默认情况下变量是全局的
  thisIsGlobal = 5  -- Camel case is common. 驼峰命名是常见的

  -- How to make a variable local: 如何使变量局部
  local line = io.read()  -- Reads next stdin line. 

  -- String concatenation uses the .. operator: 字符串连接使用..运算符
  print('Winter is coming, ' .. line)
end

-- Undefined variables return nil. 未定义的变量返回nil
-- This is not an error: 
foo = anUnknownVariable  -- Now foo = nil.

aBoolValue = false

-- Only nil and false are falsy; 0 and '' are true! 只有nil和false是假的；0和''是真的！
if not aBoolValue then print('it was false') end

-- 'or' and 'and' are short-circuited. 'or'和'and'是短路的
-- This is similar to the a?b:c operator in C/js: 这类似于C/js中的a?b:c运算符
ans = aBoolValue and 'yes' or 'no'  --> 'no'

karlSum = 0
for i = 1, 100 do  -- The range includes both ends. 范围包括两端
  karlSum = karlSum + i
end

-- Use "100, 1, -1" as the range to count down: 使用"100, 1, -1"作为范围向下计数
fredSum = 0
for j = 100, 1, -1 do fredSum = fredSum + j end

-- In general, the range is begin, end[, step]. 一般来说，范围是开始，结束[,步长]

-- Another loop construct: 另一个循环结构
repeat
  print('the way of the future')
  num = num - 1
until num == 0


----------------------------------------------------
-- 2. Functions.
----------------------------------------------------

function fib(n)
  if n < 2 then return 1 end
  return fib(n - 2) + fib(n - 1)
end

-- Closures and anonymous functions are ok: 闭包和匿名函数是可以的
function adder(x)
  -- The returned function is created when adder is 
  -- called, and remembers the value of x: 当adder被调用时，返回的函数被创建，并记住x的值
  return function (y) return x + y end
end
a1 = adder(9)
a2 = adder(36)
print(a1(16))  --> 25
print(a2(64))  --> 100

-- Returns, func calls, and assignments all work
-- with lists that may be mismatched in length. 返回、函数调用和赋值都可以使用长度不匹配的列表
-- Unmatched receivers are nil; 未匹配的接收者是nil；
-- unmatched senders are discarded. 未匹配的发送者被丢弃

x, y, z = 1, 2, 3, 4
-- Now x = 1, y = 2, z = 3, and 4 is thrown away.

function bar(a, b, c)
  print(a, b, c)
  return 4, 8, 15, 16, 23, 42
end

x, y = bar('zaphod')  --> prints "zaphod  nil nil"
-- Now x = 4, y = 8, values 15...42 are discarded.

-- Functions are first-class, may be local/global. 函数是第一类的，可以是局部的/全局的
-- These are the same:
function f(x) return x * x end
f = function (x) return x * x end

-- And so are these:
local function g(x) return math.sin(x) end
local g; g  = function (x) return math.sin(x) end
-- the 'local g' decl makes g-self-references ok. 'local g'声明使g自引用ok

-- Trig funcs work in radians, by the way. 顺便说一下，三角函数是用弧度计算的

-- Calls with one string param don't need parens: 一个字符串参数的调用不需要括号
print 'hello'  -- Works fine.


----------------------------------------------------
-- 3. Tables.
----------------------------------------------------

-- Tables = Lua's only compound data structure;  表=Lua的唯一复合数据结构；
--          they are associative arrays. 它们是关联数组。
-- Similar to php arrays or js objects, they are
-- hash-lookup dicts that can also be used as lists. 类似于php数组或js对象，它们是哈希查找字典，也可以用作列表。

-- Using tables as dictionaries / maps: 使用表作为字典/映射

-- Dict literals have string keys by default: 字典文字默认具有字符串键
t = {key1 = 'value1', key2 = false}

-- String keys can use js-like dot notation: 字符串键可以使用类似js的点表示法
print(t.key1)  -- Prints 'value1'.
t.newKey = {}  -- Adds a new key/value pair. 添加一个新的键/值对
t.key2 = nil   -- Removes key2 from the table. 从表中删除key2

-- Literal notation for any (non-nil) value as key: 任何（非nil）值作为键的文字表示法
u = {['@!#'] = 'qbert', [{}] = 1729, [6.28] = 'tau'}
print(u[6.28])  -- prints "tau"

-- Key matching is basically by value for numbers
-- and strings, but by identity for tables. 键匹配基本上是按值匹配数字和字符串，但对于表是按标识匹配
a = u['@!#']  -- Now a = 'qbert'.
b = u[{}]     -- We might expect 1729, but it's nil: 我们可能期望1729，但它是nil
-- b = nil since the lookup fails. It fails
-- because the key we used is not the same object
-- as the one used to store the original value. So
-- strings & numbers are more portable keys. 因为我们使用的键与用于存储原始值的键不同，所以失败。因此，字符串和数字是更具可移植性的键。

-- A one-table-param function call needs no parens: 一个表参数函数调用不需要括号
function h(x) print(x.key1) end
h{key1 = 'Sonmi~451'}  -- Prints 'Sonmi~451'.

for key, val in pairs(u) do  -- Table iteration.
  print(key, val)
end

-- _G is a special table of all globals. _G是所有全局变量的特殊表
print(_G['_G'] == _G)  -- Prints 'true'.

-- Using tables as lists / arrays: 使用表作为列表/数组

-- List literals implicitly set up int keys: 列表文字隐式设置int键
v = {'value1', 'value2', 1.21, 'gigawatts'}
for i = 1, #v do  -- #v is the size of v for lists. #v是v的大小
  print(v[i])  -- Indices start at 1 !! SO CRAZY! 索引从1开始！！太疯狂了！
end
-- A 'list' is not a real type. v is just a table 一个'list'不是一个真正的类型。v只是一个表
-- with consecutive integer keys, treated as a list. 具有连续整数键，被视为列表

----------------------------------------------------
-- 3.1 Metatables and metamethods. 元表和元方法
----------------------------------------------------

-- A table can have a metatable that gives the table
-- operator-overloadish behavior. Later we'll see
-- how metatables support js-prototype behavior. 一个表可以有一个元表，它给表提供了类似于运算符重载的行为。稍后我们将看到元表如何支持js-原型行为。

f1 = {a = 1, b = 2}  -- Represents the fraction a/b. 表示分数a/b
f2 = {a = 2, b = 3}

-- This would fail:
-- s = f1 + f2

metafraction = {}
function metafraction.__add(f1, f2)
  sum = {}
  sum.b = f1.b * f2.b
  sum.a = f1.a * f2.b + f2.a * f1.b
  return sum
end

setmetatable(f1, metafraction)
setmetatable(f2, metafraction)

s = f1 + f2  -- call __add(f1, f2) on f1's metatable

-- f1, f2 have no key for their metatable, unlike prototypes in js, f1，f2没有元表的键，不像js中的原型
-- so you must retrieve it as in getmetatable(f1). 所以你必须像在getmetatable(f1)中检索它一样。
-- The metatable is a normal table
-- with keys that Lua knows about, like __add. 元表是一个普通的表，具有Lua知道的键，如__add。

-- But the next line fails since s has no metatable: 但是下一行失败了，因为s没有元表
-- t = s + s
-- Class-like patterns given below would fix this. 下面给出的类似类的模式将修复这个问题。

-- An __index on a metatable overloads dot lookups: 元表上的__index重载了点查找
defaultFavs = {animal = 'gru', food = 'donuts'}
myFavs = {food = 'pizza'}
setmetatable(myFavs, {__index = defaultFavs})
eatenBy = myFavs.animal  -- works! thanks, metatable

-- Direct table lookups that fail will retry using
-- the metatable's __index value, and this recurses. 直接表查找失败将使用元表的__index值重试，这是递归的。

-- An __index value can also be a function(tbl, key)
-- for more customized lookups. 一个__index值也可以是一个函数(tbl, key)用于更自定义的查找。

-- Values of __index,add, .. are called metamethods. __index,add, ..的值称为元方法。
-- Full list. Here a is a table with the metamethod. 完整列表。这里a是一个具有元方法的表。

-- __add(a, b)                     for a + b
-- __sub(a, b)                     for a - b
-- __mul(a, b)                     for a * b
-- __div(a, b)                     for a / b
-- __mod(a, b)                     for a % b
-- __pow(a, b)                     for a ^ b
-- __unm(a)                        for -a
-- __concat(a, b)                  for a .. b
-- __len(a)                        for #a
-- __eq(a, b)                      for a == b
-- __lt(a, b)                      for a < b
-- __le(a, b)                      for a <= b
-- __index(a, b)  <fn or a table>  for a.b
-- __newindex(a, b, c)             for a.b = c
-- __call(a, ...)                  for a(...)

----------------------------------------------------
-- 3.2 Class-like tables and inheritance. 类似类的表和继承
----------------------------------------------------

-- Classes aren't built in; there are different ways
-- to make them using tables and metatables. 类不是内置的；有不同的方法使用表和元表来创建它们。

-- Explanation for this example is below it. 此示例的解释在下面。

Dog = {}                                   -- 1.

function Dog:new()                         -- 2.
  newObj = {sound = 'woof'}                -- 3.
  self.__index = self                      -- 4.
  return setmetatable(newObj, self)        -- 5.
end

function Dog:makeSound()                   -- 6.
  print('I say ' .. self.sound)
end

mrDog = Dog:new()                          -- 7.
mrDog:makeSound()  -- 'I say woof'         -- 8.

-- 1. Dog acts like a class; it's really a table. Dog就像一个类；它实际上是一个表。
-- 2. function tablename:fn(...) is the same as
--    function tablename.fn(self, ...) 函数tablename:fn(...)与函数tablename.fn(self, ...)相同
--    The : just adds a first arg called self. :只是添加了一个名为self的第一个参数。
--    Read 7 & 8 below for how self gets its value.  读7和8以了解self如何获得它的值。
-- 3. newObj will be an instance of class Dog. newObj将是类Dog的一个实例。
-- 4. self = the class being instantiated. Often
--    self = Dog, but inheritance can change it. 通常self=Dog，但继承可以改变它。
--    newObj gets self's functions when we set both
--    newObj's metatable and self's __index to self. 当我们将newObj的元表和self的__index都设置为self时，newObj会获得self的函数。
-- 5. Reminder: setmetatable returns its first arg. 提醒：setmetatable返回它的第一个参数。
-- 6. The : works as in 2, but this time we expect
--    self to be an instance instead of a class. :像2中一样工作，但这次我们期望self是一个实例而不是一个类。
-- 7. Same as Dog.new(Dog), so self = Dog in new(). 与Dog.new(Dog)相同，所以self=Dog在new()中。
-- 8. Same as mrDog.makeSound(mrDog); self = mrDog. 与mrDog.makeSound(mrDog)相同；self=mrDog。

----------------------------------------------------

-- Inheritance example:

LoudDog = Dog:new()                           -- 1.

function LoudDog:makeSound()
  s = self.sound .. ' '                       -- 2.
  print(s .. s .. s)
end

seymour = LoudDog:new()                       -- 3.
seymour:makeSound()  -- 'woof woof woof'      -- 4.

-- 1. LoudDog gets Dog's methods and variables.
-- 2. self has a 'sound' key from new(), see 3.
-- 3. Same as LoudDog.new(LoudDog), and converted to
--    Dog.new(LoudDog) as LoudDog has no 'new' key,
--    but does have __index = Dog on its metatable.
--    Result: seymour's metatable is LoudDog, and
--    LoudDog.__index = LoudDog. So seymour.key will
--    = seymour.key, LoudDog.key, Dog.key, whichever
--    table is the first with the given key.
-- 4. The 'makeSound' key is found in LoudDog; this
--    is the same as LoudDog.makeSound(seymour).

-- If needed, a subclass's new() is like the base's:
function LoudDog:new()
  newObj = {}
  -- set up newObj
  self.__index = self
  return setmetatable(newObj, self)
end

----------------------------------------------------
-- 4. Modules. 模块
----------------------------------------------------


--[[ I'm commenting out this section so the rest of
--   this script remains runnable. 我将此部分注释掉，以便脚本的其余部分保持可运行状态。
-- Suppose the file mod.lua looks like this: 假设文件mod.lua如下所示：
local M = {}

local function sayMyName()
  print('Hrunkner')
end

function M.sayHello()
  print('Why hello there')
  sayMyName()
end

return M

-- Another file can use mod.lua's functionality:
local mod = require('mod')  -- Run the file mod.lua.

-- require is the standard way to include modules. require是包含模块的标准方法
-- require acts like:     (if not cached; see below) require的作用类似于：（如果没有缓存；见下文）
local mod = (function ()
  <contents of mod.lua>
end)()
-- It's like mod.lua is a function body, so that
-- locals inside mod.lua are invisible outside it. 就像mod.lua是一个函数体，所以mod.lua内部的局部变量在外部是不可见的。

-- This works because mod here = M in mod.lua: 这是因为这里的mod=M在mod.lua中：
mod.sayHello() -- Prints: Why hello there Hrunkner 

-- This is wrong; sayMyName only exists in mod.lua: 这是错误的；sayMyName只存在于mod.lua中：
mod.sayMyName()  -- error

-- require's return values are cached so a file is
-- run at most once, even when require'd many times. require的返回值被缓存，因此一个文件最多运行一次，即使require多次。

-- Suppose mod2.lua contains "print('Hi!')". 假设mod2.lua包含"print('Hi!')"。
local a = require('mod2')  -- Prints Hi!
local b = require('mod2')  -- Doesn't print; a=b.

-- dofile is like require without caching: dofile类似于没有缓存的require：
dofile('mod2.lua')  --> Hi!
dofile('mod2.lua')  --> Hi! (runs it again)

-- loadfile loads a lua file but doesn't run it yet. loadfile加载一个lua文件，但尚未运行它。
f = loadfile('mod2.lua')  -- Call f() to run it.

-- load is loadfile for strings.
-- (loadstring is deprecated, use load instead) load是字符串的loadfile。（loadstring已弃用，请使用load代替）
g = load('print(343)')  -- Returns a function.  返回一个函数
g()  -- Prints out 343; nothing printed before now. 打印出343；现在之前没有打印任何东西。

--]]
