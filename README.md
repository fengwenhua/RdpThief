# RdpThief

基于 https://github.com/0x09AL/RdpThief 的修改，修改如下：



* 使用 `CredReadW` 拦截主机名；
* unicode 转 ansi，解决 cs 插件方框问题。



`RdpThief` 和 `ConsoleApplication1` 都是 vs 项目。其中`RdpThief` 是修改后的代码， `ConsoleApplication1` 是测试 dll 注入用的代码。



* RdpThief_x64_ansi.tmp：ansi编码，解决 cs 插件方框问题。
* RdpThief_x64_unicode.tmp：unicode编码。
* RdpThief_x64_mix.tmp：两种编码都有，防止提取出来的信息不全。

mix 版本的效果如下：

![image](https://user-images.githubusercontent.com/26518808/147683953-c4df1fa2-80e6-4c3c-be47-86b2092881bc.png)
