package thread;

import java.awt.image.BufferedImage;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.UnsupportedEncodingException;

import controller.BitmapMaker;

public class CreateBitmap extends Thread {
	private BufferedImage binImage;
	private String fileName;
	private StateCallback callback;
	
	private int width,height;
	
	public CreateBitmap(BufferedImage binImage, String fileName, StateCallback callback) {
		this.binImage = binImage;
		this.fileName = fileName;
		this.callback = callback;
	}
	
	@Override
	public void run() {
		StringBuilder builder = encodeHp39gsBitmap(binImage, fileName);
		
		saveAsTempFile(builder);
		
		builder.delete(0, builder.length());
		builder = null;
		
		BitmapMaker maker = null;
		try {
			maker = BitmapMaker.newInstance;
		} catch (java.lang.UnsatisfiedLinkError e) {
			e.printStackTrace();
			if(callback != null) {
				callback.onStateUpdated(404);
			}
			return;
		}
		
		if(maker != null) {
			maker.set_size(width, height);
			maker.set_del_tmp(1);
			int code = maker.created_bitmap_packed(fileName);
			
			if(callback != null) {
				callback.onStateUpdated(code);
			}
		}
	}
	
	private StringBuilder encodeHp39gsBitmap(BufferedImage binImage, String fileName) {
		//获取二值化后图片原始宽高
		width = binImage.getWidth();
		height = binImage.getHeight();
		int temp = 0;
		//按照8像素转换成1字节，宽需要8位对齐
		if((width % 8) != 0) {
			temp = width / 8;
			width = temp * 8;
		}
		//图片移动时4像素/次，需要4位对齐
		if((height % 4) != 0) {
			temp = height / 4;
			height = temp * 4;
		}
		//HP39AscC5 Image
		int total = width / 4 * height + 10 + fileName.length();
		int pix = 0x00000000;
		temp = 0x00000000;
		int cursor = 0;
		//文本形式保存临时文件
		StringBuilder builder = new StringBuilder(total);
		builder.append("HP39AscC");
		builder.append(String.valueOf(fileName.length()));
		builder.append(" ");
		builder.append(fileName);
		builder.append("\r");
		builder.append("\n");
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				if(binImage.getRGB(x, y) == 0xffffffff) {
					//白色--->0x00
					temp = pix >>> cursor;
					temp |= 0x00000000;
					temp = temp << cursor;
					pix |= temp; 
				}else { 
					//黑色--->0xFF
					temp = pix >>> cursor;
					temp |= 0x00000001;
					temp = temp << cursor;
					pix |= temp; 
				}
				cursor++;
				if(cursor == 4) {
					builder.append(Integer.toHexString(pix).toUpperCase());
					cursor = 0;
					pix = 0;
				}
			}
		}
		return builder;
	}
	
	private void saveAsTempFile(StringBuilder builder) {
		BufferedWriter writer = null;
		
		File file = new File(System.getProperty("user.dir")+"\\temp");
		
		if(file.exists()) {
			file.delete();
		}
		try {
			file.createNewFile();
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		
		try {
			writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file,false), "utf-8"));
			writer.write(builder.toString());
			writer.flush();
			writer.close();
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}finally {
			if(writer!=null) {
				try {
					writer.close();
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
}
