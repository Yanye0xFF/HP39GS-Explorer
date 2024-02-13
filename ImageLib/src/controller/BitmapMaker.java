package controller;

import com.sun.jna.Library;
import com.sun.jna.Native;

public interface BitmapMaker extends Library {
	BitmapMaker newInstance = (BitmapMaker)Native.loadLibrary(System.getProperty("user.dir")+"\\library\\Hp39gsBitmap.dll", BitmapMaker.class); 
	//设置图片尺寸
	public void set_size(int img_width, int img_height);
	//是否删除中间文件
	public void set_del_tmp(int state);
	//转换图片
	public int create_bitmap(String file_name);
	//转换图片-压缩格式
	public int created_bitmap_packed(String file_name);
}
