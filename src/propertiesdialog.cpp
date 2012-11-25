#include <QtGui>
#include <qtermwidget.h>

#include <QDebug>
#include "propertiesdialog.h"
#include "properties.h"
#include "config.h"

PropertiesDialog::PropertiesDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()),
            this, SLOT(apply()));
    connect(changeFontButton, SIGNAL(clicked()),
            this, SLOT(changeFontButton_clicked()));

    QStringList emulations = QTermWidget::availableKeyBindings();
    QStringList colorSchemes = QTermWidget::availableColorSchemes();

    listWidget->setCurrentRow(0);

    colorSchemaCombo->addItems(colorSchemes);
    int csix = colorSchemaCombo->findText(Properties::Instance()->colorScheme);
    if (csix != -1)
        colorSchemaCombo->setCurrentIndex(csix);

    emulationComboBox->addItems(emulations);
    int eix = emulationComboBox->findText(Properties::Instance()->emulation);
    emulationComboBox->setCurrentIndex(eix != -1 ? eix : 0 );

    /* shortcuts */
    setupShortcuts();

    /* scrollbar position */
    QStringList scrollBarPosList;
    scrollBarPosList << "No scrollbar" << "Left" << "Right";
    scrollBarPos_comboBox->addItems(scrollBarPosList);
    scrollBarPos_comboBox->setCurrentIndex(Properties::Instance()->scrollBarPos);

    /* tabs position */
    QStringList tabsPosList;
    tabsPosList << "Top" << "Bottom" << "Left" << "Right";
    tabsPos_comboBox->addItems(tabsPosList);

    // Setting windows style actions
    styleComboBox->addItem(tr("System Default"));
    styleComboBox->addItems(QStyleFactory::keys());

    setFontSample(Properties::Instance()->font);

    appOpacityBox->setValue(Properties::Instance()->appOpacity);
    //connect(appOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    termOpacityBox->setValue(Properties::Instance()->termOpacity);
    //connect(termOpacityBox, SIGNAL(valueChanged(int)), this, SLOT(apply()));

    //highlightCurrentCheckBox->setChecked(Properties::Instance()->highlightCurrentTerminal);

    historyLimited->setChecked(Properties::Instance()->historyLimited);
    historyUnlimited->setChecked(!Properties::Instance()->historyLimited);
    historyLimitedTo->setValue(Properties::Instance()->historyLimitedTo);
}

PropertiesDialog::~PropertiesDialog()
{
}

void PropertiesDialog::accept()
{
    apply();
    QDialog::accept();
}

void PropertiesDialog::apply()
{
    Properties::Instance()->colorScheme = colorSchemaCombo->currentText();
    Properties::Instance()->font = fontSampleLabel->font();//fontComboBox->currentFont();

    Properties::Instance()->emulation = emulationComboBox->currentText();

    /* do not allow to go above 99 or we lose transparency option */
    (appOpacityBox->value() >= 100) ?
            Properties::Instance()->appOpacity = 99
                :
            Properties::Instance()->appOpacity = appOpacityBox->value();

    Properties::Instance()->termOpacity = termOpacityBox->value();
    //Properties::Instance()->highlightCurrentTerminal = highlightCurrentCheckBox->isChecked();

    Properties::Instance()->scrollBarPos = scrollBarPos_comboBox->currentIndex();

    Properties::Instance()->historyLimited = historyLimited->isChecked();
    Properties::Instance()->historyLimitedTo = historyLimitedTo->value();

    saveShortcuts();

    Properties::Instance()->saveSettings();

    emit propertiesChanged();
}

void PropertiesDialog::setFontSample(const QFont & f)
{
    fontSampleLabel->setFont(f);
    QString sample("%1 %2 pt");
    fontSampleLabel->setText(sample.arg(f.family()).arg(f.pointSize()));
}

void PropertiesDialog::changeFontButton_clicked()
{
    bool ok;
    QFont f = QFontDialog::getFont(&ok, fontSampleLabel->font(), this, tr("Select Terminal Font"));
    if (ok)
        setFontSample(f);
}

void PropertiesDialog::saveShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Properties::Instance()->actions[keyValue];

        QTableWidgetItem *item = shortcutsWidget->item(x, 1);
        QKeySequence sequence = QKeySequence(item->text());
        QString sequenceString = sequence.toString();

        keyAction->setShortcut(sequenceString);
    }
}

void PropertiesDialog::setupShortcuts()
{
    QList< QString > shortcutKeys = Properties::Instance()->actions.keys();
    int shortcutCount = shortcutKeys.count();

    shortcutsWidget->setRowCount( shortcutCount );

    for( int x=0; x < shortcutCount; x++ )
    {
        QString keyValue = shortcutKeys.at(x);
        QAction *keyAction = Properties::Instance()->actions[keyValue];

        QTableWidgetItem *itemName = new QTableWidgetItem( tr(keyValue.toStdString().c_str()) );
        QTableWidgetItem *itemShortcut = new QTableWidgetItem( keyAction->shortcut().toString() );

        itemName->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );

        shortcutsWidget->setItem(x, 0, itemName);
        shortcutsWidget->setItem(x, 1, itemShortcut);
    }

    shortcutsWidget->resizeColumnsToContents();
}

void PropertiesDialog::recordAction(int row, int column)
{
    oldAccelText = shortcutsWidget->item(row, column)->text();
}

void PropertiesDialog::validateAction(int row, int column)
{
    QTableWidgetItem *item = shortcutsWidget->item(row, column);
    QString accelText = QString(QKeySequence(item->text()));

    if (accelText.isEmpty() && !item->text().isEmpty())
        item->setText(oldAccelText);
    else
        item->setText(accelText);
}
