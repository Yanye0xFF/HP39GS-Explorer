package view;

import javax.swing.*;
import java.awt.*;

public class QImageView extends JLabel {

	private static final long serialVersionUID = 1L;
	
	private Image image;

    int x = -1;
    int y = -1;
    int minX;
    int maxX;
    int minY;
    int maxY;

    public QImageView() {
    }

    @Override
    protected void paintComponent(Graphics g) {
    	if(image == null) {
    		return;
    	}
    	
        Graphics2D g2d = (Graphics2D) g.create();
        
        int currentWidth = image.getWidth(null);
        int currentHeight = image.getHeight(null);

        // 图片于容器垂直居中
        x = (getWidth() - currentWidth) / 2;
        y = (getHeight() - currentHeight) / 2;
        
        minX = (int) (-getWidth() * 0.7);
        maxX = (int) (getWidth() * 0.7);
        minY = (int) (-getHeight() * 0.7);
        maxY = (int) (getHeight() * 0.7);

        x = x < minX ? minX:x;
        x = x > maxX ? maxX:x;
        
        y = y < minY ? minY:y;
        y = y > maxY ? maxY:y;
        
        g2d.drawImage(image, x, y, null);
        g2d.dispose();
        g2d = null;
    }

    public Image getImage() {
        return image;
    }

    public void setImage(Image image) {
        this.image = image;
        this.repaint();
    }
    
    public void clearImage() {
    	this.image = null;
    }
}
