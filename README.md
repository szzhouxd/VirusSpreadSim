# VirusSpreadSim
一个用C++写的病毒传播模拟程序，使用了SDL2库。使用了四叉树碰撞检测算法，性能相对比较高。5000个点在我这么低配的电脑上完全无压力。。。真的是连我自己都惊奇了。。。

要编译此程序，你需要SDL2、SDL2_ttf和freetype。详情请参见Makefile。

要修改显示的字体，请修改Makefile中的-DFONTPATH。默认为\"C:/Windows/Fonts/msyh.ttc\"。
