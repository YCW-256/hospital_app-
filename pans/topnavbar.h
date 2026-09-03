#ifndef TOPNAVBAR_H
#define TOPNAVBAR_H

#include <QPoint>
#include <QWidget>

class QLabel;
class QTimer;

/**
 * @brief 顶部导航栏
 *
 * 白色背景，水平布局：
 *  - 左侧：圆形 LOGO + 医院中英文名称
 *  - 中间：标题文字"自助终端"
 *  - 右侧：时钟图标 + 实时日期时间（QTimer 每秒刷新）
 *
 * 由于主窗口是无边框(Frameless)窗口，这里实现了按住导航栏
 * 拖动整个窗口的功能，方便演示。
 */
class TopNavBar : public QWidget
{
    Q_OBJECT

public:
    explicit TopNavBar(QWidget *parent = nullptr);

    /** 设置中间标题文字（随当前 stack 页面切换） */
    void setCenterText(const QString &text);

signals:
    void logoClicked(); // 顶部圆形 LOGO 被点击（用于进入个人中心等）

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void initLayout();   // 构建布局
    void updateDateTime(); // 刷新日期 / 时间

    QLabel *m_titleLabel = nullptr; // 中间标题（默认"自助终端"）
    QLabel *m_timeLabel = nullptr; // 实时时间（时:分:秒）
    QLabel *m_dateLabel = nullptr; // 日期 + 星期
    QTimer *m_timer = nullptr;     // 每秒刷新定时器
    QPoint  m_dragOffset;          // 拖动窗口时的偏移量
};

/**
 * @brief 圆形 LOGO
 *
 * 用 QPainter 绘制：蓝青径向渐变圆底 + 白色医疗十字，无需任何图片资源。
 */
class CircularLogo : public QWidget
{
    Q_OBJECT

public:
    explicit CircularLogo(QWidget *parent = nullptr);

signals:
    void clicked(); // 点击圆形 LOGO

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
};

#endif // TOPNAVBAR_H
