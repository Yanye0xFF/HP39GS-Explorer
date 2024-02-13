package ui;

import java.awt.Dimension;
import java.awt.EventQueue;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.WindowStateListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.filechooser.FileSystemView;

import model.WindowPosition;
import thread.BinaryResultCallback;
import thread.CreateBitmap;
import thread.StateCallback;
import view.QButton;
import view.QImageView;
import view.Toast;

import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JScrollPane;
import javax.swing.JToggleButton;
import javax.swing.JTabbedPane;
import javax.swing.JRadioButton;
import javax.swing.JSpinner;
import javax.swing.JComboBox;

public class StartFrame extends JFrame implements ActionListener{

	private static final long serialVersionUID = 1L;

	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					StartFrame frame = new StartFrame();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	private QImageView imageView;
	private JScrollPane scrollPane;
	private JTextField textField;
	
	private Toast toast =null;

	private static Frame windowFrame;
	private static WindowPosition current;
	
	private StateCallback stateCallback;
	private CreateBitmap createBitmapThread;
	
	private BinaryResultCallback resultCallback;
	private ExecutorService cachedThreadPool = Executors.newCachedThreadPool();
	
	public StartFrame() {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setTitle("Hp39gs图片格式转换");
		Dimension dimension = Toolkit.getDefaultToolkit().getScreenSize();
		setLocation(dimension.width/2-800/2, dimension.height/2-400/2);
		setSize(800,400);
		
		setPreferredSize(new Dimension(800, 400));
		setMinimumSize(new Dimension(800,400));
		
		JPanel contentPane = new JPanel();
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		contentPane.setLayout(null);
		setContentPane(contentPane);
		
		Font font = new Font("宋体", Font.PLAIN, 14);
		
		windowFrame = getFrames()[0];
		
		current = new WindowPosition();
		current.x = getX();
		current.y = getY();
		current.width = 600;
		current.height = 400;
		
		toast = new Toast(windowFrame, current, "",Toast.SHORT);
		
		JLabel lbFilePath = new JLabel("图片路径:");
		lbFilePath.setBounds(10, 10, 70, 15);
		lbFilePath.setFont(font);
		contentPane.add(lbFilePath);
		
		textField = new JTextField();
		textField.setBounds(76, 7, 285, 21);
		textField.setEditable(false);
		contentPane.add(textField);
		textField.setColumns(10);
		
		QButton btnSelect = new QButton("...");
		btnSelect.setBounds(361, 7, 40, 21);
		contentPane.add(btnSelect);
		
		scrollPane = new JScrollPane();
		scrollPane.setBounds(10, 35, 564, 316);
		contentPane.add(scrollPane);
		
		imageView = new QImageView();
		scrollPane.setViewportView(imageView);
		
		QButton btnPreview = new QButton("单色预览");
		btnPreview.setText("黑白转换");
		btnPreview.setBounds(636, 166, 85, 21);
		contentPane.add(btnPreview);
		
		QButton btnExport = new QButton("导出");
		btnExport.setBounds(705, 7, 69, 21);
		contentPane.add(btnExport);
		
		JComboBox<String> comboBox = new JComboBox<String>();
		comboBox.setFont(new Font("宋体", Font.PLAIN, 12));
		comboBox.setBounds(462, 7, 85, 21);
		comboBox.addItem("迭代法");
		comboBox.addItem("OTSU");

		contentPane.add(comboBox);
		
		JLabel lblNewLabel = new JLabel("算法：");
		lblNewLabel.setFont(font);
		lblNewLabel.setBounds(413, 10, 45, 15);
		contentPane.add(lblNewLabel);
		
		JLabel lblGeshi = new JLabel("格式：");
		lblGeshi.setFont(font);
		lblGeshi.setBounds(557, 10, 45, 15);
		contentPane.add(lblGeshi);
		
		JComboBox<String> comboBox_1 = new JComboBox<String>();
		comboBox_1.setFont(new Font("宋体", Font.PLAIN, 12));
		comboBox_1.addItem("bmp-dib");
		comboBox_1.addItem("compress");
		comboBox_1.setBounds(605, 7, 85, 21);
		contentPane.add(comboBox_1);
		
		windowFrame.addComponentListener(new ComponentAdapter(){
			@Override public void componentResized(ComponentEvent e){
				onWindowResize(windowFrame);
			}
		});
		
		windowFrame.addWindowStateListener(new WindowStateListener () {
			public void windowStateChanged(WindowEvent state) {
				int newState=state.getNewState();
				switch (newState) {
					case Frame.NORMAL:
						onWindowResize(windowFrame);
						break;
					case Frame.MAXIMIZED_BOTH:
						onWindowResize(windowFrame);
						break;
					default:
						break;
				}
			}
		});
		
		addMouseMotionListener(new MouseMotionAdapter() {
			@Override
			public void mouseMoved(MouseEvent e) {
				current.x = getX();
				current.y = getY();
			}
		});
		
		btnSelect.addActionListener(this);
		btnPreview.addActionListener(this);
		btnExport.addActionListener(this);
		
		stateCallback = new StateCallback() {
			@Override
			public void onStateUpdated(int stateCode) {
				if(stateCode == 0) {
					toast.showAtLocation(current, "转换成功");
				}else if(stateCode == -1) {
					toast.showAtLocation(current, "删除中间文件失败");
				}else if(stateCode == -100) {
					toast.showAtLocation(current, "打开中间文件失败");
				}else if(stateCode == -200) {
					toast.showAtLocation(current, "打开输出文件失败");
				}else if(stateCode == -101) {
					toast.showAtLocation(current, "关闭输出文件失败");
				}else if(stateCode == -201) {
					toast.showAtLocation(current, "关闭中间文件失败");
				}else if(stateCode == 404) {
					toast.showAtLocation(current, "加载链接库失败");
				}
			}
		};
		
		resultCallback = new BinaryResultCallback() {
			@Override
			public void onFinished(BufferedImage binImage) {
				imageView.clearImage();
				imageView.setImage(binImage);
				hasBinary = true;
				toast.showAtLocation(current, "二值化完成");
			}
		};
		
	}
	
	/**
	 * 窗口尺寸改变
	 * */
	private void onWindowResize(Frame frame){
		Dimension dim = frame.getSize();
		int width = (int)dim.getWidth();
		int height = (int)dim.getHeight();
		//更新当前窗口位置信息
		current.width = width;
		current.height = height;
		
		scrollPane.setSize(new java.awt.Dimension(width - 35, height - 85));
		imageView.setSize(new java.awt.Dimension(width - 35, height-85));
		imageView.revalidate();
		
		dim = null;
	}
	
	private File selectedFile = null;
	private boolean hasBinary = false;
	@Override
	public void actionPerformed(ActionEvent e) {
		if(e.getActionCommand().equals("...")) {
			selectedFile = showFileSelectorDialog();
			if(selectedFile != null) {
				textField.setText(selectedFile.getAbsolutePath());
				displayImage(selectedFile,false);
				hasBinary = false;
				toast.showAtLocation(current, "图片正确加载");
			}else {
				//toast.showAtLocation(current, "未选择图片文件");
			}
		}else if(e.getActionCommand().equals("黑白转换")) {
			if(selectedFile != null) {
				displayImage(selectedFile,true);
			}else {
				toast.showAtLocation(current, "请先打开图片");
			}
		}else if(e.getActionCommand().equals("导出")) {
			BufferedImage binImage = (BufferedImage)imageView.getImage();
			if(binImage != null) {
				if(!hasBinary) {
					toast.showAtLocation(current, "请转换图片");
					return;
				}
				
				String defaultName = selectedFile.getName().substring(0, selectedFile.getName().indexOf('.'));
				
				String inputName = (String)JOptionPane.showInputDialog(windowFrame, "输入在Hp39gs显示的文件名(英文/数字 10字内)", "导出数据",
						JOptionPane.QUESTION_MESSAGE, null, null, defaultName);
				
		        if(inputName != null && !inputName.isEmpty()) {
		        	if(inputName.length() > 9) {
		        		toast.showAtLocation(current, "文件名限制10字内");
		        		return;
		        	}
		        	createBitmapThread = new CreateBitmap(binImage, inputName, stateCallback);
		        	cachedThreadPool.execute(createBitmapThread);
		        }
			}else {
				toast.showAtLocation(current, "请先打开图片");
			}
		}
	}
	
	private JFileChooser fileChooser;
	private FileNameExtensionFilter fileFilter;
	private JLabel fileDialogTitle;
	private File showFileSelectorDialog() {
		if(fileChooser == null) {
			fileChooser=new JFileChooser();
			fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
			fileChooser.setCurrentDirectory(FileSystemView.getFileSystemView().getHomeDirectory());
			fileChooser.setApproveButtonText("确定");
			fileChooser.setDialogTitle("选择图片");
			
			fileDialogTitle = new JLabel();
			fileFilter = new FileNameExtensionFilter("图片文件(*.bmp,*.png,*.jpg)", "bmp","png","jpg");
			fileChooser.setFileFilter(fileFilter);
		}
		int option = fileChooser.showOpenDialog(fileDialogTitle);
		if(option == JFileChooser.APPROVE_OPTION) {
			return fileChooser.getSelectedFile();
		}
		return null;
	}
	
	/**
	 * 显示图片
	 * @param file 图片文件
	 * @param binary 是否二值化
	 * */
	private void displayImage(File file,boolean binary) {
		BufferedImage rawImage = null;
		try {
			rawImage = ImageIO.read(file);
		} catch (IOException e) {
			e.printStackTrace();
		}
		if(rawImage != null) {
			if(binary) {
				cachedThreadPool.execute(new thread.BinaryImage(rawImage, resultCallback));
			}else {
				imageView.clearImage();
				imageView.setImage(rawImage);
			}
		}
	}
}
