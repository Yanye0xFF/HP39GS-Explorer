package thread;

import java.awt.image.BufferedImage;

/**
 * 二值化算法思路：遍历图像的所有像素点，计算每个像素点的灰度值。通过迭代法收敛得到一个最佳阈值，
 * 灰度值大于最佳阈值的像素点设为白色，灰度值小于最佳阈值的像素点设为黑色。
 * 迭代法获取最佳阈值思路：
 * 1.设最小灰度值为Gmin，最大灰度值为Gmax，阈值初始化为T(0)=(Gmin+Gmax)/2。
 * 2.以阈值T(k)将图像分割为前景和背景，求出整个前景像素的平均灰度值Gf和整个背景像素的平均灰度值Gb，此时阈值T(k)=(Gf+Gb)/2 (k=0,1,2...);
 * 3.若此时T(k)=T(k+1)，那么此时收敛，得到最佳阈值。否则回到步骤2，直到阈值收敛到某一个值。
 * */
public class BinaryImage extends Thread {
	
	private BufferedImage binImage;
	private BufferedImage rawImage;
	private BinaryResultCallback callback;
	
	public BinaryImage(BufferedImage rawImage, BinaryResultCallback callback) {
		this.rawImage = rawImage;
		this.callback = callback;
	}

	@Override
	public void run() {
		
		int w = rawImage.getWidth();
		int h = rawImage.getHeight();
		
		int total = w * h;
		
		int[] gray = new int[total];
        int[] rgbArray = new int[total];
        
        binImage = new BufferedImage(w, h, BufferedImage.TYPE_BYTE_BINARY);
        
        rawImage.getRGB(0, 0, w, h, rgbArray, 0, w);
        
        for (int index = 0; index < total; index++) {
            int red = (rgbArray[index] & 0x00FF0000) >>> 16;
            int green = (rgbArray[index] & 0x0000FF00) >>> 8;
            int blue = rgbArray[index] & 0x000000FF;
            //经验公式   Gray = R * 0.299 + G * 0.587 + B * 0.114
            gray[index] = (int) ((float) red * 0.3 + (float) green * 0.59 + (float) blue * 0.11);
        }
        
        //求出最大灰度值zmax和最小灰度值zmin
        int Gmax = gray[0], Gmin = gray[0];
        for (int index = 0; index < total; index++) {
            if (gray[index] > Gmax) {
                Gmax = gray[index];
            }
            if (gray[index] < Gmin) {
                Gmin = gray[index];
            }
        }
        
        //获取灰度直方图
        int i, j, t, count1 = 0, count2 = 0, sum1 = 0, sum2 = 0;
        int bp, fp;
        int[] histogram = new int[256];
        for (t = Gmin; t <= Gmax; t++) {
            for (int index = 0; index < total; index++) {
                if (gray[index] == t)
                    histogram[t]++;
            }
        }
        
        /*
         * 迭代法求出最佳分割阈值
         * */
         int T = 0;
         int newT = (Gmax + Gmin) / 2;//初始阈值
         while (T != newT) {
        	//求出背景和前景的平均灰度值bp和fp
             for (i = 0; i < T; i++) {
                 count1 += histogram[i];//背景像素点的总个数
                 sum1 += histogram[i] * i;//背景像素点的灰度总值
             }
             bp = (count1 == 0) ? 0 : (sum1 / count1);//背景像素点的平均灰度值

             for (j = i; j < histogram.length; j++) {
                 count2 += histogram[j];//前景像素点的总个数
                 sum2 += histogram[j] * j;//前景像素点的灰度总值
             }
             fp = (count2 == 0) ? 0 : (sum2 / count2);//前景像素点的平均灰度值
             T = newT;
             newT = (bp + fp) / 2;
         }
         
         int finestYzt = newT; //最佳阈值
         
         int index = 0;
         for(int y = 0; y < h; y++) {
        	 for(int x = 0; x < w; x++) {
        		 if (gray[index++] > finestYzt) {
        			 binImage.setRGB(x, y, 0xFFFFFFFF);
        		 }else {
        			 binImage.setRGB(x, y, 0xFF000000);
        		 }
        	 }
         }
         
         if(callback != null) {
        	 callback.onFinished(binImage);
         }
	}	
}
