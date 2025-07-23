问题已修复！

修复内容：
1. 在 DVSDataSource::stopRecord() 方法中添加了 recordingConfig->clearCurrentSession() 调用
2. 在 CameraCapture::stopRecord() 方法中也添加了相同的调用
3. 修改了 DVSDataSource::startRecord() 方法，让它总是创建新的录制会话，而不是重用现有会话

问题原因：
- 之前DVS录制结束后没有清理当前会话路径(currentSessionPath)
- 下次录制时会检查是否有现有会话，如果有就重用，导致所有录制都保存到第一个目录
- DV相机没有这个问题是因为它总是强制创建新会话

现在的行为：
- 每次录制结束后都会清理会话状态
- 每次开始录制都会创建新的时间戳目录
- DVS和DV数据会保存到对应的新目录中

测试方法：
1. 启动程序
2. 多次开始和停止录制
3. 检查录制目录，应该看到多个不同的时间戳文件夹
4. 每个文件夹都应该包含对应录制的DVS数据

修复完成！
