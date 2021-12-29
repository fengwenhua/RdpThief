# RdpThief

基于 https://github.com/0x09AL/RdpThief 的修改，修改如下：



* 使用 `CredReadW` 拦截主机名；
* unicode 转 ansi，解决 cs 插件方框问题。



`RdpThief` 和 `ConsoleApplication1` 都是 vs 项目。其中`RdpThief` 是修改后的代码， `ConsoleApplication1` 是测试 dll 注入用的代码。


* RdpThief_x64_unicode.tmp：unicode编码（老版本）。

  ![image](https://user-images.githubusercontent.com/26518808/147684182-403f2dc3-fbb0-478f-861c-9478709975de.png)

* RdpThief_x64_ansi.tmp：ansi编码，解决 cs 插件方框问题（修改版）。

  ![image](https://user-images.githubusercontent.com/26518808/147684256-28314149-7c19-4c27-8a8a-b62d0ad4e454.png)

* RdpThief_x64_mix.tmp：两种编码都有，防止提取出来的信息不全。

  ![image](https://user-images.githubusercontent.com/26518808/147683953-c4df1fa2-80e6-4c3c-be47-86b2092881bc.png)
