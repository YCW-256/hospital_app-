#ifndef ICONFACTORY_H
#define ICONFACTORY_H

#include <QPixmap>
#include <QSize>
#include <QString>

/**
 * @brief 图标工厂
 *
 * 所有图标均以"内联 SVG 字符串"形式内置在代码中（Qt 内置 svg），
 * 通过 QSvgRenderer 统一渲染为指定尺寸、透明背景的 QPixmap，
 * 不依赖任何外部图片资源，方便部署与二次修改。
 */
namespace IconFactory {

    /** 将一段内联 SVG 渲染为指定尺寸的 QPixmap（背景透明） */
    QPixmap renderSvg(const QString &svgString, const QSize &size);

    /* ---------- 首页六个功能按钮图标（白色线条，viewBox 100x100） ---------- */
    const QString &appointmentSvg();   // 预约挂号：日历 + 加号
    const QString &paymentSvg();       // 门诊缴费：手掌 + 人民币硬币
    const QString &feeQuerySvg();      // 费用查询：放大镜 + 人民币符号
    const QString &cardIssueSvg();     // 病历卡片
    const QString &medicineQuerySvg(); // 药费查询：药箱
    const QString &rechargeSvg();      // 门诊充值：环形人民币
    const QString &personSvg();        // 个人中心：人形轮廓

    /* ---------- 顶部导航栏时钟图标 ---------- */
    const QString &clockSvg();

    /* ---------- AI 快速问诊页症状图标（蓝色圆形 + 白色符号） ---------- */
    const QString &symptomFeverSvg();   // 发热咳嗽：体温计
    const QString &symptomStomachSvg(); // 肠胃不适：肠道蠕动
    const QString &symptomSkinSvg();    // 皮肤问题：手掌
    const QString &symptomTongueSvg();  // 舌苔健康：口舌

    /* ---------- 底部症状输入框麦克风（深蓝色线条，适配白底输入框） ---------- */
    const QString &micBlueSvg();

    /* ---------- 通用图标 ---------- */
    const QString &downloadSvg();  // 下载（箭头入托盘）
    const QString &certifiedSvg(); // 认证（绿色圆形 + 白色对勾）
    const QString &micSvg();       // 麦克风
    const QString &searchSvg();    // 放大镜（搜索）
}

#endif // ICONFACTORY_H
