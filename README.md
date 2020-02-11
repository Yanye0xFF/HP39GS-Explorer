## HP39GS File Explorer 简单介绍
`一个为HP39GS图形计算器提供额外的中文文本阅读及图片浏览功能的Aplet应用程序`

中文阅读：文本编码使用GB2312-80字符集，字体像素大小为8*8；GB2312-80基本集共收入汉字6763个和非汉字图形字符682个，基本满足了汉字的计算机处理需要，它所收录的汉字已经覆盖中国大陆99.75%的使用频率。

图片浏览：图片格式为专有格式（需要使用配套的图片转换工具），由于HP39GS存储空间，转换后的图片仅支持黑白两色（暂不支持16灰度图片，这将导致文件大小大幅增长）。使用像素压缩模式，有效减小文件占用空间，压缩模式下典型值压缩率为0.7（与上一版类BMP格式相比）。

## 文件目录概述
1. release：安装于计算器的程序，HP39GS安装Explorer.bin，HP40/48安装Explorer.hp（LIB275L.lib ARM运行库都需要安装）。

2. rom：hp39gs_v2.22_raw.bin 原版2.22固件；hp39gs_v2.22_update.bin 添加了GB2312-80字库固件，其他功能不受影响。

3. src：程序源码。

4. tools：HP39gsBitmapMaker.jar 图片转换工具，需安装64位jre，且仅支持64位系统。

## Explorer 如何安装？
首先需要确认您已正确安装Conn3x_Chineseusbconnectkit及其HPCalculator_usb_driver，在win10下需要关闭驱动签名验证。
##### 更新中文字库固件
1. 在计算器开机状态下移除主电池，同时按住`+ -`键，并使用USB先将计算器与电脑连接。

2. 计算器通电后，保持按住`+ -`键至少`3秒`后松开，此时计算器进入bootloader模式。

3. 按下数字键盘`1`，进入`UPDATE CODE`。

4. 再次按下数字键盘`1`，进入`USB`, 此时计算器进入固件下载模式。

5. 打开`HP39gs Calculator Connectivity Kit`，等待计算器与PC握手正确，出现文件列表。

6. 选择`ROM`-->`下载计算器只读存储器ROM`

7. 浏览到固件`hp39gs_v2.22_update.bin`，确认即可。

8. 下载完毕按下`Enter`重启计算器即成功。

##### 下载 Explorer 应用程序
1. 打开`HP39gs Calculator Connectivity Kit`，等待计算器与PC握手正确，出现文件列表。

2. 在 `Connectivity Kit` 左侧文件树界面进入应用程序所在文件夹，需要注意的是`hp39dir.000`，  
   `hp39dir.cur`，`Explorer.bin` 在同一文件夹。

3. 计算器按下`APLET`进入程序列表界面，选中`RECV`，在弹出的Dialog中选择`HP39/40`，点击`OK`。

4. 此时PC端 `Connectivity Kit`将弹出文件选择对话框，选中`Explorer.bin`下载即可。

5. 下载进度条完毕后，重复第三步，下载`LIB275L.lib` ARM TOOLBOX支持库，没有此支持库程序将无法运行。

##### 下载 文本文件
1. 使用您顺手的文本编辑器，新建文本文件，将编码模式调整为GB2312 936）。

2. 在文件首行进行格式填充`HP39AscC4 Test`。其中`HP39AscC`为HP39GS内部记事本应用文件头，后面的数字为文件名长度，空一格，输入文件名（文件名仅支持ASCII字符）。

3. 填充文件头完毕，换行，写入您期望的文本文件内容即可，最后保存，文件名随意可无后缀。

4. 打开`HP39gs Calculator Connectivity Kit`，等待计算器与PC握手正确，出现文件列表。

5. 计算器按下`APLET`进入程序列表界面，选中`RECV`，在弹出的Dialog中选择`USB Disk driv...`，点击`OK`。

6. PC端连接软件将等待一会后建立连接（此期间创建临时`hp39dir.000`，`hp39dir.cur`文件）。

7. 等待一会后，计算器弹出文件名输入框，在此输入该文件的文件名（PC端显示的文件名）即可。

8. 注意：在 `步骤2` 写入的文件名是最终计算器文件列表显示的名称，而在 `步骤7` 计算器端输入的文件名是该文件显示的名称，即在 `步骤3` 保存的文件名，这两个文件名彼此独立。
##### 下载 图片文件
1. 使用 `HP39gsBitmapMaker.jar ` 图片转换工具将PNG JPG BMP...转换成自有格式。

2. 参照  `下载 文本文件 ` 的  `步骤5 ` -->  `步骤7 `

转换工具主界面  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/conv_main.png)  
第一步：打开图片文件   
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/conv_open.png)  
图片打开状态  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/conv_step1.png)  
第二步：转换  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/conv_step2.png)  
第三步：存储(目标文件位于程序运行目录)  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/conv_output.png) 

## Explorer 功能使用方法？

* 文件列表
    *  列出设备内文件，支持文件名及大小显示
    * 使用`上/下/左/右` 方向键进行换页
* 文本阅读
    * 使用`上/下/左/右` 方向键进行换页
    * 使用`5/2` 数字键盘进行换页（数字键盘离手较近，便于操作）
    * 使用`Enter` 键打开/关闭屏幕，方便离开时隐藏显示
    * 文本文件大小无限制（取决于设备存储空间）
* 图片浏览
    * 使用`上/下/左/右` 方向键移动显示区域
    * 使用`5/2/1/3` 数字键盘移动显示区域
    * 使用`Enter` 键打开/关闭屏幕
    * 垂直/水平当前位置指示器
    * 最大支持图片大小 `65535*65535`
* 其他功能
    * 在使用本程序时按下`ON`可快速关机，下次开机将会跳过主界面，直接进入本程序
## 程序预览
(如果下列预览图无法正常显示，请连接至外网后刷新页面)  
程序列表  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/explorer_aplet_list.jpg)  
应用内文件列表  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/explorer_file_list.jpg)  
图片浏览  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/explorer_image_view.jpg)  
文本浏览  
![aplet-list](https://raw.githubusercontent.com/Yanye0xFF/PictureBed/master/images/hp39explorer/explorer_text_view.jpg)  
## BUG提交
在使用中有任何问题，欢迎反馈给我，可以用以下联系方式跟我交流

* 邮件(www.wanganshi#gmail.com, 把#换成@)