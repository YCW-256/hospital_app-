#ifndef DOCTORCARD_H
#define DOCTORCARD_H

#include <QString>
#include <QWidget>

class CircularAvatar;
class QEnterEvent;
class QEvent;
class QLabel;
class QMouseEvent;

/**
 * @brief 医生信息卡（用于挂号页选择医生）
 *
 * 卡片内容：圆形头像（CircularAvatar） + 姓名 + 科室。
 * 持有 doctor_info_use 的各个属性（id / name / time / department）作为成员变量，
 * 便于后续【确认预约】提交时直接读取。
 * 支持 hover 高亮与"选中"状态（蓝色描边 + 浅蓝底），
 * 点击卡片发出 clicked() 信号，由外部管理互斥选中。
 */
class DoctorInfoCard : public QWidget
{
    Q_OBJECT

public:
    /**
     * @param id         医生 ID（doctor_info_use.id）
     * @param name       医生姓名（doctor_info_use.name）
     * @param dept       所属科室（doctor_info_use.department）
     * @param time       值班时段（doctor_info_use.time，0=上午 1=下午 2=晚上）
     * @param avatarPath 头像图片路径（暂统一为测试头像 icons/doctor2.jpg）
     * @param parent     父控件
     */
    explicit DoctorInfoCard(int id,
                            const QString &name,
                            const QString &dept,
                            int time,
                            const QString &avatarPath,
                            QWidget *parent = nullptr);

    int id() const;            // 医生 ID（doctor_info_use.id）
    QString name() const;      // 医生姓名（doctor_info_use.name）
    QString dept() const;      // 所属科室（doctor_info_use.department）
    int time() const;          // 值班时段（doctor_info_use.time，0=上午 1=下午 2=晚上）
    bool isSelected() const;

public slots:
    void setSelected(bool selected); // 设置选中状态（刷新样式）

signals:
    void clicked(); // 点击卡片（选择该医生）

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void updateStyle(); // 依据选中/悬停状态刷新样式表

    // 医生值班信息（doctor_info_use 各属性，供后续预约提交使用）
    int     m_id   = -1;   // 医生 ID
    int     m_time = -1;   // 值班时段（0=上午 1=下午 2=晚上）
    QString m_name;        // 医生姓名
    QString m_dept;        // 所属科室

    CircularAvatar *m_avatar    = nullptr;
    QLabel         *m_nameLabel = nullptr;
    QLabel         *m_deptLabel = nullptr;
    bool            m_selected  = false; // 是否选中
    bool            m_hover     = false; // 鼠标是否悬停
};

#endif // DOCTORCARD_H
