#ifndef UISTYLE_H
#define UISTYLE_H

#include <QString>

class QLabel;
class QPushButton;
class QTableView;
class QWidget;

/**
 * @brief 统一界面样式
 *
 * 集中管理三个子页面共用的配色常量与控件工厂方法，
 * 保证与首页风格完全统一（大尺寸触摸按钮、圆角、深蓝表头等）。
 */
namespace UIStyle {

/* ---------- 配色常量 ---------- */
const QString kPrimaryBlue       = QStringLiteral("#2E86DE"); // 主蓝（实心按钮）
const QString kPrimaryBlueHover  = QStringLiteral("#4A9CE8"); // 主蓝 hover
const QString kPrimaryBluePress  = QStringLiteral("#1F6FB8"); // 主蓝 pressed
const QString kDeepBlue          = QStringLiteral("#1F4E79"); // 深蓝（表头/描边/文字）
const QString kLightBorder       = QStringLiteral("#C8D4E2"); // 浅边框
const QString kTableAlt          = QStringLiteral("#F4F8FC"); // 表格隔行底色
const QString kTableGrid         = QStringLiteral("#D5DEEA"); // 表格边框
const QString kHeaderSep         = QStringLiteral("#3A6EA5"); // 表头列分隔线

/* ---------- 控件工厂 ---------- */

/** 页面顶部大标题（白色加粗，左上对齐） */
void stylePageTitle(QLabel *label);

/** 子页面透明背景，透出主窗口蓝色渐变（需在页面构造时调用） */
void styleTransparentPage(QWidget *page);

/** 解析资源图片路径（当前目录 / 程序目录 / 程序上级目录 依次尝试） */
QString resolveImagePath(const QString &relativePath);

/** 白色圆角卡片容器（需调用方 addWidget 放入布局） */
void styleCard(QWidget *card);

/** 蓝色实心圆角大按钮（适合触摸） */
QPushButton *createPrimaryButton(const QString &text, QWidget *parent = nullptr);

/** 白色底 + 深蓝描边圆角大按钮（适合触摸） */
QPushButton *createOutlineButton(const QString &text, QWidget *parent = nullptr);

/**
 * 通用表格样式：深蓝表头 + 白字、隔行底色、隐藏行号、禁止编辑/选中。
 * 调用方需再 setColumnCount / setHorizontalHeaderLabels 并填充数据。
 */
void styleTable(QTableView *table);

} // namespace UIStyle

#endif // UISTYLE_H
