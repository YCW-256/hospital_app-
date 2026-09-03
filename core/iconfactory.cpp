#include "iconfactory.h"

#include <QPainter>
#include <QSvgRenderer>

namespace IconFactory {

namespace {
// 通用白色线条样式：描边、圆头、圆角连接
const QString kWhiteStroke =
    QStringLiteral("fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"5\" "
                   "stroke-linecap=\"round\" stroke-linejoin=\"round\"");
} // namespace

QPixmap renderSvg(const QString &svgString, const QSize &size)
{
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);

    QSvgRenderer renderer(svgString.toUtf8());
    if (renderer.isValid()) {
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&painter);
    }
    return pixmap;
}

/* 预约挂号：日历 + 中央加号 */
const QString &appointmentSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<rect x=\"14\" y=\"20\" width=\"72\" height=\"68\" rx=\"10\"/>"
        "<path d=\"M14 38 H86\"/>"
        "<path d=\"M32 14 V26\"/>"
        "<path d=\"M68 14 V26\"/>"
        "<path d=\"M50 48 V76\"/>"
        "<path d=\"M36 62 H64\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 门诊缴费：张开的手掌 + 上方人民币硬币 */
const QString &paymentSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<path d=\"M24 54 C24 44 32 38 40 38 L62 38 C70 38 76 44 76 54 L76 68 "
        "C76 77 68 80 60 80 L40 80 C31 80 24 74 24 68 Z\"/>"
        "<path d=\"M30 38 V28\"/>"
        "<path d=\"M42 36 V22\"/>"
        "<path d=\"M54 36 V23\"/>"
        "<path d=\"M66 38 V29\"/>"
        "<path d=\"M26 42 C18 45 16 52 19 56\"/>"
        "<circle cx=\"50\" cy=\"15\" r=\"11\"/>"
        "<path d=\"M45 11 L50 16 L55 11\"/>"
        "<path d=\"M43 14 H57\"/>"
        "<path d=\"M50 16 V23\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 费用查询：放大镜镜片内含人民币符号 */
const QString &feeQuerySvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<circle cx=\"42\" cy=\"40\" r=\"23\"/>"
        "<path d=\"M59 57 L80 78\"/>"
        "<path d=\"M35 34 L42 43 L49 34\"/>"
        "<path d=\"M34 41 H50\"/>"
        "<path d=\"M42 43 V55\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 自助发卡：带医疗十字的照片病历卡片 */
const QString &cardIssueSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<rect x=\"14\" y=\"16\" width=\"72\" height=\"68\" rx=\"8\"/>"
        "<rect x=\"22\" y=\"24\" width=\"34\" height=\"34\" rx=\"4\"/>"
        "<path d=\"M39 32 V50\"/>"
        "<path d=\"M30 41 H48\"/>"
        "<path d=\"M62 28 H80\"/>"
        "<path d=\"M62 36 H74\"/>"
        "<path d=\"M22 64 H80\"/>"
        "<path d=\"M22 72 H66\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 药费查询：带十字标志的药箱 */
const QString &medicineQuerySvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<rect x=\"24\" y=\"20\" width=\"52\" height=\"62\" rx=\"8\"/>"
        "<path d=\"M38 20 V12 H62 V20\"/>"
        "<path d=\"M50 34 V62\"/>"
        "<path d=\"M34 48 H66\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 门诊充值：环形圆 + 人民币符号 */
const QString &rechargeSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<circle cx=\"50\" cy=\"50\" r=\"36\"/>"
        "<circle cx=\"50\" cy=\"50\" r=\"25\" stroke-width=\"4\" stroke-dasharray=\"7 5\"/>"
        "<path d=\"M41 42 L50 51 L59 42\"/>"
        "<path d=\"M39 49 H61\"/>"
        "<path d=\"M50 51 V66\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 个人中心：人形轮廓（头 + 肩） */
const QString &personSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" %1>"
        "<circle cx=\"50\" cy=\"35\" r=\"18\"/>"
        "<path d=\"M20 86 C20 64 34 56 50 56 C66 56 80 64 80 86\"/>"
        "</svg>").arg(kWhiteStroke);
    return svg;
}

/* 顶部导航栏时钟图标（深蓝色线条） */
const QString &clockSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\" stroke=\"#2B6CB0\" stroke-width=\"7\" stroke-linecap=\"round\">"
        "<circle cx=\"50\" cy=\"50\" r=\"38\"/>"
        "<path d=\"M50 50 V30\"/>"
        "<path d=\"M50 50 L66 60\"/>"
        "</svg>");
    return svg;
}

/* ============ AI 快速问诊页症状图标（蓝色圆形 + 白色符号） ============ */

namespace {
// 症状图标通用：蓝色圆形底
const QString kSymptomCircle = QStringLiteral(
    "<circle cx=\"50\" cy=\"50\" r=\"46\" fill=\"#2E86DE\"/>");
// 症状图标通用：白色符号描边
const QString kSymptomStroke =
    QStringLiteral("fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"8\" "
                   "stroke-linecap=\"round\" stroke-linejoin=\"round\"");
} // namespace

/* 发热咳嗽：体温计 */
const QString &symptomFeverSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\">%1"
        "<path d=\"M50 20 V52\" %2/>"
        "<circle cx=\"50\" cy=\"66\" r=\"13\" %2/>"
        "</svg>").arg(kSymptomCircle, kSymptomStroke);
    return svg;
}

/* 肠胃不适：肠道蠕动曲线 */
const QString &symptomStomachSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\">%1"
        "<path d=\"M28 44 C28 68 42 74 56 70 C68 66 72 54 61 48\" %2/>"
        "<path d=\"M38 62 C42 70 54 72 61 64\" %2/>"
        "</svg>").arg(kSymptomCircle, kSymptomStroke);
    return svg;
}

/* 皮肤问题：张开的手掌 */
const QString &symptomSkinSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\">%1"
        "<circle cx=\"50\" cy=\"63\" r=\"11\" %2/>"
        "<path d=\"M42 51 L37 27\" %2/>"
        "<path d=\"M50 51 V24\" %2/>"
        "<path d=\"M58 51 L63 27\" %2/>"
        "<path d=\"M62 63 L77 50\" %2/>"
        "</svg>").arg(kSymptomCircle, kSymptomStroke);
    return svg;
}

/* 舌苔健康：嘴巴 + 舌头 */
const QString &symptomTongueSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\">%1"
        "<path d=\"M27 46 Q50 64 73 46\" %2/>"
        "<path d=\"M40 55 C45 67 55 67 60 55\" %2/>"
        "</svg>").arg(kSymptomCircle, kSymptomStroke);
    return svg;
}

/* 底部症状输入框麦克风：深蓝色线条，适配白色输入框背景 */
const QString &micBlueSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\" stroke=\"#2B6CB0\" stroke-width=\"7\" stroke-linecap=\"round\" "
        "stroke-linejoin=\"round\">"
        "<rect x=\"36\" y=\"12\" width=\"28\" height=\"52\" rx=\"14\"/>"
        "<path d=\"M22 44 C22 60 34 72 50 72 C66 72 78 60 78 44\"/>"
        "<path d=\"M50 72 V88\"/>"
        "<path d=\"M34 88 H66\"/>"
        "</svg>");
    return svg;
}

/* 下载图标：箭头落入托盘（深蓝色线条，用于白色卡片右上角） */
const QString &downloadSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\" stroke=\"#2B6CB0\" stroke-width=\"7\" stroke-linecap=\"round\" "
        "stroke-linejoin=\"round\">"
        "<path d=\"M50 14 V58\"/>"
        "<path d=\"M27 36 L50 60 L73 36\"/>"
        "<path d=\"M16 84 H84\"/>"
        "</svg>");
    return svg;
}

/* 认证图标：绿色圆形 + 白色对勾（用于医保卡号右侧） */
const QString &certifiedSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" fill=\"none\">"
        "<circle cx=\"50\" cy=\"50\" r=\"46\" fill=\"#2E9E6B\"/>"
        "<path d=\"M30 52 L44 66 L72 36\" stroke=\"#FFFFFF\" stroke-width=\"9\" "
        "stroke-linecap=\"round\" stroke-linejoin=\"round\"/>"
        "</svg>");
    return svg;
}

/* 麦克风图标（语音助手按钮） */
const QString &micSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"7\" stroke-linecap=\"round\" "
        "stroke-linejoin=\"round\">"
        "<rect x=\"36\" y=\"12\" width=\"28\" height=\"52\" rx=\"14\"/>"
        "<path d=\"M22 44 C22 60 34 72 50 72 C66 72 78 60 78 44\"/>"
        "<path d=\"M50 72 V88\"/>"
        "<path d=\"M34 88 H66\"/>"
        "</svg>");
    return svg;
}

/* 放大镜搜索图标（语音输入右侧蓝色圆形按钮） */
const QString &searchSvg()
{
    static const QString svg = QStringLiteral(
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 100\" "
        "fill=\"none\" stroke=\"#FFFFFF\" stroke-width=\"8\" stroke-linecap=\"round\">"
        "<circle cx=\"42\" cy=\"42\" r=\"26\"/>"
        "<path d=\"M62 62 L84 84\"/>"
        "</svg>");
    return svg;
}

} // namespace IconFactory
