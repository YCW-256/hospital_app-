#ifndef MODIFYINFOPAGE_H
#define MODIFYINFOPAGE_H

#include <QWidget>

/**
 * @brief 修改信息页（QStackedWidget 索引 7）
 *
 * 由个人中心【修改信息】按钮进入。白色圆角卡片内可编辑姓名/手机号码/医保卡号，
 * 卡片头部右侧为【语音助手】按钮，点击弹出悬浮 AI 信息助手。
 */
class ModifyInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit ModifyInfoPage(QWidget *parent = nullptr);

signals:
    void backRequested();            // 【返回】点击，切回个人中心
    void voiceAssistantRequested();  // 【语音助手】点击，弹出 AI 助手

private:
    void initLayout(); // 构建页面布局
};

#endif // MODIFYINFOPAGE_H
