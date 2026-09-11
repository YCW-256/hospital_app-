# 医院自助终端（patient_port2）

## 项目概述

- 技术栈：**Qt 6.10.3 + C++17**，MinGW 工具链（`Desktop Qt 6.10.3 llvm-mingw 64-bit` 套件），qmake 构建。
- 功能：医院自助终端界面，无边框主窗口 + 顶部导航栏 + QStackedWidget 多页面切换，演示首页 6 个功能入口（预约挂号 / 门诊缴费 / 费用查询 / 个人中心 / 药费查询 / 门诊充值）。

## 目录结构约定

- `pans/` —— **界面文件**统一放在此目录（非界面文件按类别放同级文件夹，见下）：
  - 登录/注册（独立顶层窗口，启动流程用）：`loginwindow.*` 登录、`registerwindow.*` 注册账号。
  - 主窗口：`mainwindow.*`；`topnavbar.*` 顶部导航栏；`functionbutton.*` 首页圆角功能按钮。
  - 业务子页：`paymentpage.*` 待缴费用清单、`appointmentpage.*` 预约挂号、`feepage.*` 费用查询、`aiconsultpage.*` AI 快速问诊、`personalcenterpage.*` 个人中心、`modifyinfopage.*` 修改信息。
  - 悬浮 AI 助手弹窗：`aiassistantpopup.*`。
- `pans/childs/` —— 自定义控件模块，随界面一起归入 `pans/` 管理；已编译：`circularavatar.*`（圆形头像）、`doctorcard.*`（医生信息卡，持有 `doctor_info_use` 各属性 `id`/`name`/`time`/`department` 成员变量，供后续预约提交直接读取）、`chatbubble.*`（纯代码聊天气泡，自适应宽度：短文本窄气泡、长文本按封顶宽换行，复刻原 ChatBom 算法）、`appointmentconfirmpopup.*`（挂号确认弹窗 `AppointmentConfirmPopup`，无边框置顶悬浮，展示医生姓名/科室/挂号日期（当天）/就诊时段，含【取消】与【确认挂号】按钮，确认发 `confirmed()`）。注：`chatbom.*` 依赖 .ui/.qrc，不符合本工程"纯代码"约束，未编译，由 `chatbubble.*` 替代。
- `core/` —— 基础工具（与 `pans/` 同级）：`iconfactory.*` 内联 SVG 图标工厂；`uistyle.*` 统一界面样式助手（卡片/按钮/表格样式与配色常量）。
- `agent/` —— AI 智能体（与 `pans/` 同级）：`deepseekagent.*`（DeepSeek 智能体基类 `DeepSeekAgent`，带对话记忆）+ `myagent.*`（个人信息助手 `MyAgent`）+ `consultagent.*`（AI 快速问诊智能体 `ConsultAgent`，与 MyAgent 分离、独立记忆）。
  - **职责分离**：基类 `DeepSeekAgent::ask()` 只负责调用 API 获取大模型返回的**原始数据**（返回 `QByteArray`，网络失败/超时返回空），不解析；数据解析交给各派生类，派生类同名 `ask()` 隐藏基类方法，解析后返回 `QString` 供界面使用（对话记忆的"用户:"提问由基类记录，"小A:"回复由派生类解析后调用受保护的 `recordAssistantReply()` 记录）。
- `audio/` —— 语音播放（与 `pans/` 同级）：`ttsplayer.*`（Windows SAPI 语音播放，移植自 patient_port 原工程）。
- `voice/` —— 语音识别（与 `pans/` 同级，**语音识别相关新增文件统一放此目录**）：
  - `audiorecorder.*` 录音器 `AudioRecorder`（Qt6 `QAudioSource`：16kHz/单声道/Int16 写入内存缓冲，提供 Int16→Float32 归一化样本 `getFloat32AudioData()`），移植自参考工程 sense_voice_demo。
  - `sensevoiceengine.*` 识别引擎 `SenseVoiceEngine`（Sherpa-ONNX C API 封装 SenseVoice 离线模型：cpp 内 `extern "C" #include "c-api.h"`；`loadModel(dir)` 加载 `model.int8.onnx`+`tokens.txt`，`recognizeAudio(std::vector<float>)` 识别 16kHz Float32，结果剥离 `<|…|>` 语言/情感标记），移植自 sense_voice_demo。
  - `speechrecognizer.*` 语音识别控制器 `SpeechRecognizer`（**全局单例**）：把「按住说话→录音→松手→识别→回文字」封成 `startListening(requester)`/`stopListening()`，识别引擎 `moveToThread` 独立工作线程（不卡 UI）、录音在主线程；模型**首次按住时把"加载"排队到引擎线程**（加载与识别同线程 FIFO，故录音立即可开始、无需等待加载完成，约 1~3s 后模型就绪）；`recognized(requester,text)`/`errorOccurred(requester,msg)` 携带发起者指针，**界面只在 `requester==自身` 时处理**，保证两个聊天框互不串扰。
  - 依赖子目录（自参考 demo 复制，随工程自包含）：`sherpa/include/`（`c-api.h`/`cargs.h`，已加 INCLUDEPATH）、`sherpa/lib/`（导入库 `sherpa-onnx-c-api.lib` + 运行 DLL `sherpa-onnx-c-api.dll`/`onnxruntime.dll`/`onnxruntime_providers_shared.dll`/`cargs.dll`）、`model/`（SenseVoice 模型 `model.int8.onnx` 约 239MB + `tokens.txt`）。**239MB 模型建议加入 .gitignore，避免入库。**
- `device/` —— 硬件设备（与 `pans/` 同级）：
  - `devicecamera.*` 舌苔检测摄像头（RV1106）TCP 视频流接收器 `DeviceCamera`，移植自参考工程 `rv1106_test01` 的 VideoReceiver（帧协议：魔数 `LZDZ` + 数据长度 + 行 + 列 + 原始像素，BGR888/灰度自动识别，独立线程运行、断开/连接失败经单发定时器 `m_reconnectTimer` 每 2 秒自动重连）。
  - `cameraserial.*` 舌苔摄像头串口控制器 `CameraSerial`，移植自参考工程 SerialWorker 的下行部分：打开 `COM9`(115200) 成功自动下发 `0x0001`（自动推视频），下行固定 4 字节帧 `0x5A|CMD_H|CMD_L|0xA5`（公共方法 `sendDownFrame(quint16)` 统一发送）；**`triggerTongueDetect()`**（public slot，跨线程下发 `0x0010`）触发设备进入**单帧推理**——当前画面一旦检测到舌苔即上行回报 6 字节帧 `0x5B|CMD_H|CMD_L|类别编码(0x00~0x04，0xFF=未识别)|置信度(0x00~0x64=0~100%)|0xA5`；`onReadyRead()` 以**缓冲 + 状态机**解析上行帧（`m_rxBuffer` 拼接防半包/粘包，帧尾校验、误同步丢弃首字节重找帧头），解析成功发 `tongueDetected(int classId, float confidence)` 信号（置信度按 /0x64 解码），另发 `logMessage` 打印 HEX；每次 `readyRead` 也先打印收到的上行裸字节 HEX 作为调试日志（用于确认设备是否真的回报）；**`resumeVideo()`**（public slot，下发 `0x0001`）解除单帧检测后的冻结、恢复实时推流（配合【再次检测】按钮重新预览/再采集）。打开失败每 3 秒自动重试。类别编码→舌苔名称（灰黑苔/镜面舌/薄白苔/白腻苔/黄腻苔，按索引 0~4）映射由界面层 `AIConsultPage::onTongueDetected()` 负责（须与设备端 labels.txt / `GetTongueClassCode` 顺序一致）。
- `main.cpp` —— 程序入口，留在项目根目录。
- `MyTcp/`（协议 `protecol.h`、数据缓存 `CData`、TCP 客户端 `SocketLink`）、`Task/`（响应处理任务，如 `GetDoctorInfoTask` 把医生信息写入 `CData::m_doctor_info`）、`Tool/`（工具）为通信模块，**已加入编译**（见 `patient_port2.pro`）；`icons/` 存放界面用到的 jpg/png 静态图片。`CData::m_doctor_info`（`static vector<doctor_info_use>`，字段 `id`/`name`/`time`/`department`）是服务器返回的**今日值班医生**列表，其中 `time` 为值班时段：0=上午、1=下午、2=晚上（与 `ddd/hos` 的 `myTime[3]` 约定一致）。当前登录患者信息：`CData::m_id`/`CData::m_name` 由登录响应 `PATIENT_LOGIN_RESP` 写入（`SocketLink::recv_data()`）；`CData::m_phone`（QString）为**当前登录手机号**，在 `LoginWindow::onLogin()` 发出登录请求时写入，个人中心页读取展示。`CData::m_register_doctor`（`doctor_info_use`）+ `CData::m_register_date`（`QString`，yyyy-MM-dd）为**已确认挂号暂存**：预约挂号页弹窗点【确认挂号】时由 `AppointmentPage::onAppointmentConfirmed()` 写入，供后续挂号提交流程读取。

## 页面索引与交互规则

QStackedWidget 页面索引（在 `MainWindow::initStackedPages()` 中注册）：

| 索引 | 页面 | 对应首页按钮 |
|---|---|---|
| 0 | 首页 | — |
| 1 | 待缴费用清单页 `PaymentPage` | 门诊缴费 |
| 2 | 预约挂号页 `AppointmentPage` | 预约挂号 |
| 3 | 费用查询页 `FeeQueryPage` | 费用查询 |
| 4 | AI 快速问诊页 `AIConsultPage` | 药费查询（原占位位改为实际页面） |
| 5 | 门诊充值（占位页） | 门诊充值 |
| 6 | 个人中心页 `PersonalCenterPage` | 首页【个人中心】按钮（原自助发卡位）/ 导航栏 LOGO |
| 7 | 修改信息页 `ModifyInfoPage` | 个人中心【修改信息】 |

交互规则：
1. 导航栏中间标题随当前页面切换：`TopNavBar::setCenterText()` + `QStackedWidget::currentChanged` 联动，首页显示「自助终端」。
2. 首页按钮点击切换索引：预约挂号→2、门诊缴费→1、费用查询→3、个人中心→6、药费查询→4（AI 快速问诊）、门诊充值→5（映射见 `MainWindow::createHomePage()`）。
3. **每个页面都必须有返回按钮**（首页除外）：各子页提供【上一步/返回】按钮（可置于页面顶部或底部），点击一律切回首页（索引 0），通过各页 `backRequested()` 信号实现。
4. 控件样式统一走 `UIStyle` 助手：白色圆角卡片、深蓝表头 + 白字、表格隔行底色、大尺寸触摸圆角按钮。
5. 预约挂号页（索引 2）：
   - 左侧【选择科室】为互斥可选中科室网格（内科/外科/儿科/妇产科/骨科/眼科/耳鼻喉科/皮肤科，2 列），**默认选中内科**（第一个），点击打印 `选择XXX科`（qDebug 输出）；**每次科室切换（`QPushButton::toggled` 且勾选）触发 `flush()`** 重筛医生；科室可增删，修改 `AppointmentPage::initLayout()` 中 `depts` 列表。
   - 右侧：【上午/下午】互斥时段按钮（默认上午），**每次时段切换（`toggled` 且勾选）触发 `flush()`** 重筛；【选择医生】为 3 列医生信息卡网格（`DoctorInfoCard`，含头像/姓名/科室，头像暂统一 `icons/doctor2.jpg`，卡头像 84×84），网格放在 `QScrollArea` 内可纵向滚动。
   - **医生卡片由服务器数据驱动**：首页点击【预约挂号】即调 `AppointmentPage::getDoctorInfo()` 请求今日值班医生；响应经 `GetDoctorInfoTask` 写入 `CData::m_doctor_info`，再由 `SocketLink::get_doctor_info_success` 触发 `AppointmentPage::flush()`（连接见 `MainWindow::init_data_connect()`）。
   - **`flush()` 逻辑**：先清空右侧所有旧卡片，再遍历 `CData::m_doctor_info`，筛选科室 `department` 等于当前选中科室（未选科室则不过滤）且时段 `time` 等于当前时段（0=上午/1=下午，未选则不过滤）的记录，逐个 `new DoctorInfoCard(id, name, department, time, avatarPath, …)` 放入 3 列网格；点击卡片互斥选中，当前选中卡片存入成员 `m_selectedDoctor`，并**输出卡片内医生信息** `选择医生：id=… 姓名=… 科室=… 时段=time`（`qDebug`，取自卡片的 `id()`/`name()`/`dept()`/`time()`），**随后调用 `showDoctorConfirm(dc)` 弹出挂号确认弹窗**。医生头像 `m_avatarPath` 暂统一为 `icons/doctor2.jpg`。
   - **挂号确认弹窗**：`AppointmentConfirmPopup`（`childs/appointmentconfirmpopup.*`），无边框置顶悬浮（复用 AIAssistantPopup 写法），由 `AppointmentPage::showDoctorConfirm()` 懒创建并**复用单实例** `m_confirmPopup`（父对象为主窗口 `window()`，**居中于主窗口正中央**）；弹窗展示该医生的挂号信息（医生姓名 / 科室 / 挂号日期取当天 / 就诊时段上午|下午|晚上，由 `time` 0|1|2 映射），含【取消】（隐藏弹窗、不破坏已选中卡片）与【确认挂号】。点【确认挂号】发 `confirmed()` 触发 `AppointmentPage::onAppointmentConfirmed()` 槽（`private slots`）：把 `m_selectedDoctor` 的医生信息写入 `CData::m_register_doctor`（`doctor_info_use`）+ `CData::m_register_date`（当天 `yyyy-MM-dd`），仅准备数据、暂不发网络请求。`AppointmentPage` 重写 `hideEvent()`：页面切走时同步隐藏弹窗。
6. 个人中心页（索引 6）：由首页【个人中心】按钮（原"自助发卡"位，`IconFactory::personSvg` 人形图标）或导航栏左上角 LOGO（`TopNavBar::logoClicked`）进入；居中白色大圆角卡片内 4 行个人信息（姓名/手机号码/医保卡号/认证状态），医保卡号右侧带绿色对勾认证图标（`IconFactory::certifiedSvg`）。**数据跟随当前登录用户**：姓名取 `CData::m_name`（登录响应返回）、手机号取 `CData::m_phone`（登录时写入）并**中间打码**显示（`maskPhone()`：138****5678），医保卡号暂无独立数据源暂与手机号展示一致，认证状态默认"已认证"；四行值标签保存为成员，页面每次进入（重写 `showEvent()` → `refreshInfo()`）重新从 CData 刷新。卡片右下角【修改信息】（信号 `modifyRequested`）进入修改信息页；页面底部左下为提示文字、右下【返回首页】切回首页。
7. 修改信息页（索引 7）：由个人中心【修改信息】进入，返回切回个人中心（索引 6）；卡片头部右侧【语音助手】按钮（`IconFactory::micSvg`）弹出悬浮 AI 信息助手（`AIAssistantPopup`，无边框、置顶、居中于主窗口、单实例，可拖动标题栏）；弹窗结构：医生头像标题栏「AI信息助手」+ 聊天气泡区（`ChatBubble`，AI 白底靠左/用户蓝底靠右）+ 三个快捷按钮（查看认证状态/修改手机号/修改医保信息，本地应答）+ 语音输入区（输入框占位"请输入问题，或按住麦克风说话" + 蓝色圆形麦克风按钮 + 蓝色圆形发送按钮）：输入框回车或点发送按钮走自由文本；**按住麦克风按钮说话、松手自动识别，识别出的文字直接调用 MyAgent 发送（无需再点发送按钮）**（语音识别见 `voice/` 模块与"语音识别接入"）；输入框自由文本 / 识别文字均调用 `MyAgent`（DeepSeek API，key 在 `agent/myagent.cpp`，带对话记忆：每轮"用户:/小A:"记入 `m_history` 随请求发送，超 20 条自动裁剪）；AI 回复（欢迎语 / 快捷应答 / AI 回复）自动语音播报（`TtsPlayer`，Windows SAPI，.pro 已链接 `-lole32 -lsapi -luuid`）；每次朗读使用 `SPF_ASYNC | SPF_PURGEBEFORESPEAK`，新语句会打断上一句未播完的语音。
8. AI 快速问诊页（索引 4）：由首页【药费查询】按钮进入（原占位位改为实际页面），返回切回首页（索引 0），**聊天式问诊**：左侧白色圆角视频卡片（顶部一行【舌苔摄像头】提示 + 摄像头【打开/关闭】开关 `m_toggleBtn`，`QPushButton#cameraToggleBtn` 可勾选、默认未勾选：勾选后 `QLabel#videoLabel` 铺满显示硬件上传的舌苔图像，未勾选/未收到帧则纯黑；其右侧另有**绿色【舌苔检测】按钮 `m_detectBtn`**（`QPushButton#cameraDetectBtn`，`clicked` → `onDetectTongue()`：跨线程投递串口 `triggerTongueDetect()` 下发 `0x0010` 触发一次单帧舌苔检测，设备检测到舌苔即上行回报；回报经串口线程解析 `tongueDetected(int,float)` 信号回主线程 → `AIConsultPage::onTongueDetected()` 按类别编码映射中文舌苔名并 **qDebug 打印** `[舌苔检测] 类别=XXX(编码N) 置信度=NN.N%`，**同时组装一句话 `通过边缘模型检测用户的舌头为XXX，请你给出建议` 交给 `submitText()` 发给 AI 医生**（与打字/语音共用发送逻辑，AI 回复照常语音播报；类别越界即设备端 0xFF 未识别时只打印日志、不打扰 AI）。其再右侧是**橙色【再次检测】按钮 `m_resumeBtn`**（`QPushButton#cameraResumeBtn`，`clicked` → `onResumeVideo()`：跨线程投递串口 `resumeVideo()` 下发 `0x0001`，解除设备单帧检测后的冻结、恢复实时预览，以便重新对焦后再点【舌苔检测】采集）。其再右侧是**紫色【上传】按钮 `m_uploadBtn`**（`QPushButton#cameraUploadBtn`，`clicked` → `AIConsultPage::onUploadImage()`：把当前画面（`m_latestFrame`，与摄像头开关无关；**尚未收到帧时为 640×640 纯黑图，黑图也照传**）经 `makeUploadImage()` **等比放大 + 居中裁剪**成 640×640（`kUploadImageSize`=640，不变形、不带圆角），再 `encodeUploadImage()` 编码为图片字节流（优先 **JPEG quality=90**，JPEG 插件缺失时回退 Qt 内置 **PNG**），存入成员 **`m_uploadImageData`** 并 `qDebug` 打印 `[舌苔上传] 已生成上传图片：宽×高 格式 共 N 字节`，随后 emit **`uploadRequested(const QByteArray&)`** 供外部取用；**发送网络请求的代码留空待补**（`onUploadImage()` 末尾留 `TODO(发送)` 标记，由调用方实现）。四个按钮（打开/关闭、舌苔检测、再次检测、上传）宽度统一由 120 缩为 110 以保证单行排得下。视频承载 `QLabel#videoLabel` 为纯代码、不依赖 multimedia 模块）+ 视频区下方单行 4 个常见症状快捷按钮（发热咳嗽/肠胃不适/皮肤问题/舌苔健康，互斥选中，`QButtonGroup`，图标为蓝色圆形 SVG：`symptomFeverSvg`/`symptomStomachSvg`/`symptomSkinSvg`/`symptomTongueSvg`），点击即作为一条用户消息发送给 AI 医生；右侧白色圆角聊天窗口（深蓝标题栏：医生头像 `icons/doctor2.jpg` + 标题 + ● 在线，中部 `ChatBubble` 气泡滚动区，浅灰底）+【返回首页】；两列顶部均无标题、底部【返回首页】（高 `kBottomRowHeight`=64）与左侧症状按钮行等高，使**视频区域与聊天框高度对齐**；底部通栏：蓝色圆形语音麦克风按钮 `m_voiceBtn`（`IconFactory::micSvg`，**按住说话 → 松手自动识别，识别文字直接作为一条用户消息发给 AI 医生，无需点【开始问诊】**）+ 症状输入框（`QLineEdit`，占位「请描述症状，不超过 300 字」，`setMaxLength(300)`）+【开始问诊】（回车或点击发送）。普通输入走 `onSend()`（内部统一调 `submitText()`），语音识别走 `SpeechRecognizer::recognized` → `submitText()`，两者共用发送逻辑（语音识别见 `voice/` 模块与"语音识别接入"）。问答走独立的 AI 快速问诊智能体 `ConsultAgent`（`consultagent.*`，继承 `DeepSeekAgent` 基类，医疗问诊系统提示词 + 独立对话记忆），AI 欢迎语与每条回复自动语音播报（`TtsPlayer`）；**注意：欢迎语播报用 `QTimer::singleShot(0,…)` 延迟到事件循环启动后执行**，SAPI 语音依赖消息泵，若在 `QApplication::exec()` 前同步调用会阻塞导致主窗口无法显示（kEnableLogin=0 直进主界面时曾因此不出窗口）。两个智能体已分离：`MyAgent`（个人信息助手，供 AI 信息助手弹窗）与 `ConsultAgent`（医疗问诊，供 AI 快速问诊页）各自单例、独立对话记忆，后续可各自独立演进。
   - **舌苔检测摄像头集成**（参考上位机 `rv1106_test01` 的 VideoReceiver 提取实现逻辑）：新目录 `device/`（与 `pans/` 同级）的 `device/devicecamera.*` 定义 `DeviceCamera` TCP 视频流接收器，帧协议为 魔数 `LZDZ`(4B) + 图像数据长度 dataLen(4B 大端) + 行 rows(4B 大端) + 列 cols(4B 大端) + dataLen 字节原始像素；像素判定 `dataLen==rows*cols*3` → BGR888（按 RGB888 读入后 `rgbSwapped()` 还原颜色）、`dataLen==rows*cols*1` → 灰度图；单帧上限 5MB、超限丢弃，解析期间保留末尾 3 字节防魔数跨包截断，运行中(`m_running`)断开或 `connectToHost` 失败后由单发重连定时器 `m_reconnectTimer` 每 2 秒自动重连（去重）。IP/端口按 rv1106_test01 的 ui 预设**写死** `10.1.1.144:6868`（`AIConsultPage::initCamera()`）。`AIConsultPage` 构造时在 `initCamera()` 里同时启动**两条独立线程**并 `moveToThread` 对应对象（**初始化即连好设备**）：① 串口线程 `CameraSerial`（串口 `COM9`/115200 写死）`start()` 打开成功后自动下发 `0x0001` 让摄像头开始推流，并连接其 `tongueDetected(int,float)` → `AIConsultPage::onTongueDetected()`（主线程槽 qDebug 打印舌苔结果）；② 网络线程 `DeviceCamera` `start()` 连接 `10.1.1.144:6868` 收图。`frameReady(QImage)` 队列连接回主线程缓存到 `m_latestFrame`；`onToggleCamera(bool)` 切 `m_cameraOn` 并刷新画面：打开且已收到帧时 `refreshVideoLabel()` 以 `makeCoverPixmap()`（等比放大 + 居中裁剪，不变形）**铺满** `QLabel#videoLabel`，否则 `m_videoLabel->clear()` 露出纯黑底色；页面析构先 `QMetaObject::invokeMethod(m_camera, "stop", BlockingQueuedConnection)` 再 `quit()/wait()` 停线程再释放。label 黑色圆角样式写代码内、圆角 `kVideoLabelRadius`=16 与样式一致。
9. **启动流程（登录/注册）**：`main.cpp` 中登录开关 `kEnableLogin` 置 1 时调用 `MainWindow::enableLogin()`：`LoginWindow`/`RegisterWindow` 作为 **MainWindow 的子遮罩**（不加入 QStackedWidget，`setGeometry(rect())` 覆盖整个主窗口，`resizeEvent` 同步尺寸），启动即显示登录遮罩，登录成功才隐藏露出主界面。`LoginWindow`：蓝色渐变 + 白色圆角卡片（手机号码 + 密码 + 红色错误提示 +【登录】+ 底部【还没有账号？立即注册】），校验（11 位手机号 + 非空密码）通过发 `loginSucceeded()` 隐藏遮罩；右上角×关闭顶层主窗口退出。`RegisterWindow`：标题「注册账号」，布局仿修改信息页（姓名 / 手机号码 / 密码（掩码）/ 医保卡号 / 认证状态（未认证）），卡片头部右侧【语音助手】弹出 `AIAssistantPopup`（与修改信息页共用 MyAgent，单实例，用 `window()->geometry().center()` 居中于主窗口），底部【退出】/【确定注册】返回登录遮罩并预填手机号（`LoginWindow::setPhone`）。`kEnableLogin` 置 0 时直接显示主界面、不创建遮罩。登录/注册均非 QStackedWidget 子页，是主窗口的覆盖子控件。

10. **语音识别接入（`voice/` 模块，两个聊天框共用同一套）**：AI 快速问诊页（索引 4）底部麦克风 与 AI 信息助手弹窗 `AIAssistantPopup` 底部麦克风，均为**蓝色圆形按住说话的 `QPushButton`**：`pressed()` → `SpeechRecognizer::instance()->startListening(this)`，`released()` → `stopListening()`。两处都在**首次按住时懒绑定**识别单例的 `recognized(requester,text)` / `errorOccurred(requester,msg)` 到自身（信号带发起者指针，只有 `requester==this` 才处理），避免两个聊天框互相收到对方识别结果。识别出非空文字 → 直接走各自发送逻辑自动发送（问诊页 `submitText()`→`ConsultAgent`，弹窗 `submitText()`→`MyAgent` 并照常发 `full_text`），**不经过输入框、无需再点发送按钮**；识别失败（模型缺失 / 无麦克风 / 没录到声音）以 AI 侧气泡提示、不播报。`SpeechRecognizer` 全局单例、懒创建（首次按住才建引擎与线程，模型首次按住在引擎线程异步加载、不阻塞录音）；页面析构调用 `cancelFor(this)` 释放进行中的录音 / 识别。模型目录解析：优先编译期宏 `VOICE_MODEL_DIR`（.pro 注入为 `$$PWD/voice/model`），其次 exe 目录 / 上级目录的 `voice/model` 兜底。工程新增 `QT += multimedia`（录音用）并链接 `voice/sherpa/lib/sherpa-onnx-c-api.lib`、构建后把 DLL 复制到 exe 目录（见 .pro 末尾 win32 块）。

## 代码约束（必须遵守）

1. **纯代码构建**：不使用 `.ui` 文件、不使用 Qt Designer、不使用 `.qrc` 资源文件；界面一律用 C++ 代码构建。
2. **图标**：统一使用**内联 SVG 字符串 + `IconFactory::renderSvg()`** 渲染，禁止引入外部图片资源文件；新增图标时在 `core/iconfactory.cpp` 中添加 SVG 字符串并暴露访问函数。
3. **样式**：所有样式用 **Qt StyleSheet 写在代码内**，不单独拆 `.qss` 文件，不写外部样式资源。
4. **架构**：主窗口沿用【顶部导航栏 TopNavBar + 中部 QStackedWidget】架构；新增功能页作为 QStackedWidget 子页，并在 `MainWindow::initStackedPages()` 中注册。
5. **主窗口**：无边框（`Qt::FramelessWindowHint`），蓝色渐变背景，固定 1280×800；**右上角保留关闭按钮**；导航栏支持鼠标拖动窗口。
6. **编译配置**：新增源文件必须同步更新 `patient_port2.pro` 的 `SOURCES` / `HEADERS`；涉及 Qt 模块时更新 `QT +=` 一行。
7. **代码风格**：注释与界面文案使用中文；类名大驼峰、文件/变量小写开头；保持注释清晰、组件职责单一。
8. **目录分类**：`pans/` 只放界面文件；非界面文件按类别放入与 `pans/` **同级别**的文件夹（`core/` 基础工具、`agent/` 智能体、`audio/` 语音、`device/` 硬件设备等），没有对应文件夹时新建。

## 工作方式（Claude 必须遵守）

1. **只写代码，不编译不运行**：任务只负责编写 / 修改代码，不做编译、运行、截图等验证操作；验证由用户自行进行。
2. **每次任务后更新本文件**：每完成一个任务，把本次涉及的约定、目录变化、新增组件 / 模块、改动要点同步记录到本 CLAUDE.md，保持约束文档始终与代码现状一致。

## 构建方式

- 推荐：Qt Creator 打开 `patient_port2.pro`，选择 `Desktop Qt 6.10.3 llvm-mingw 64-bit` 套件构建运行。
- 命令行编译：

```bash
export PATH="/f/QT6/Tools/llvm-mingw1706_64/bin:/f/QT6/6.10.3/llvm-mingw_64/bin:/f/QT6/Tools/mingw1310_64/bin:$PATH"
qmake patient_port2.pro -spec win32-clang-g++
mingw32-make -j8
```

产物输出到 `bin/patient_port2.exe`。

- 语音识别依赖随工程自包含在 `voice/`：`voice/sherpa/`（头/库/DLL，.pro 已配置 INCLUDEPATH/LIBS 并在构建后把运行 DLL 复制进 exe 目录）、`voice/model/`（SenseVoice 模型约 239MB，模型目录经 `VOICE_MODEL_DIR` 宏注入代码，运行时也兜底查找 exe 旁/上级 `voice/model`）。模型体积大，建议在 `.gitignore` 忽略 `voice/model/`。


