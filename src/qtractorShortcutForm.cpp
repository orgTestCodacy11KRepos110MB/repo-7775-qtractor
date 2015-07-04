// qtractorShortcutForm.cpp
//
/****************************************************************************
   Copyright (C) 2005-2015, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorShortcutForm.h"

#include "qtractorActionControl.h"

#include "qtractorMidiControlObserverForm.h"

#include "qtractorOptions.h"

#include <QAction>
#include <QMenu>

#include <QMessageBox>
#include <QPushButton>
#include <QHeaderView>

#include <QPainter>
#include <QLineEdit>
#include <QToolButton>
#include <QKeyEvent>


//-------------------------------------------------------------------------
// qtractorShortcutTableItem

class qtractorShortcutTableItem : public QTableWidgetItem
{
public:

	// Constructors.
	qtractorShortcutTableItem(const QIcon& icon, const QString& sText)
		: QTableWidgetItem(icon, sText)
		{ setFlags(flags() & ~Qt::ItemIsEditable); }
	qtractorShortcutTableItem(const QString& sText)
		: QTableWidgetItem(sText)
		{ setFlags(flags() & ~Qt::ItemIsEditable); }
	qtractorShortcutTableItem(const QKeySequence& shortcut)
		: QTableWidgetItem(shortcut.toString()) {}
};


//-------------------------------------------------------------------------
// qtractorShortcutTableItemEdit

// Shortcut key to text event translation.
void qtractorShortcutTableItemEdit::keyPressEvent ( QKeyEvent *pKeyEvent )
{
	const Qt::KeyboardModifiers modifiers
		= pKeyEvent->modifiers();

	int iKey = pKeyEvent->key();

	if (modifiers == Qt::NoModifier) {
		switch (iKey) {
		case Qt::Key_Return:
			emit editingFinished();
			return;
		case Qt::Key_Escape:
			emit editingCanceled();
			return;
		}
	}

	if (iKey >= Qt::Key_Shift && iKey < Qt::Key_F1) {
		QLineEdit::keyPressEvent(pKeyEvent);
		return;
	}

	if (modifiers & Qt::ShiftModifier)
		iKey |= Qt::SHIFT;
	if (modifiers & Qt::ControlModifier)
		iKey |= Qt::CTRL;
	if (modifiers & Qt::AltModifier)
		iKey |= Qt::ALT;

	QLineEdit::setText(QKeySequence(iKey).toString());
}


//-------------------------------------------------------------------------
// qtractorShortcutTableItemEditor

qtractorShortcutTableItemEditor::qtractorShortcutTableItemEditor (
	QWidget *pParent ) : QWidget(pParent)
{
	m_pItemEdit = new qtractorShortcutTableItemEdit(/*this*/);

	m_pToolButton = new QToolButton(/*this*/);
	m_pToolButton->setFixedWidth(18);
	m_pToolButton->setText("X");

	QHBoxLayout *pLayout = new QHBoxLayout();
	pLayout->setSpacing(0);
	pLayout->setMargin(0);
	pLayout->addWidget(m_pItemEdit);
	pLayout->addWidget(m_pToolButton);
	QWidget::setLayout(pLayout);
	
	QWidget::setFocusPolicy(Qt::StrongFocus);
	QWidget::setFocusProxy(m_pItemEdit);

	QObject::connect(m_pItemEdit,
		SIGNAL(editingFinished()),
		SLOT(finish()));
	QObject::connect(m_pItemEdit,
		SIGNAL(editingCanceled()),
		SLOT(cancel()));
	QObject::connect(m_pToolButton,
		SIGNAL(clicked()),
		SLOT(clear()));
}


// Shortcut text accessors.
void qtractorShortcutTableItemEditor::setText ( const QString& sText )
{
	m_pItemEdit->setText(sText);
}


QString qtractorShortcutTableItemEditor::text (void) const
{
	return m_pItemEdit->text();
}


// Shortcut text clear/toggler.
void qtractorShortcutTableItemEditor::clear (void)
{
	if (m_pItemEdit->text() == m_sDefaultText)
		m_pItemEdit->clear();
	else
		m_pItemEdit->setText(m_sDefaultText);

	m_pItemEdit->setFocus();
}


// Shortcut text finish notification.
void qtractorShortcutTableItemEditor::finish (void)
{
	const bool bBlockSignals = m_pItemEdit->blockSignals(true);
	emit editingFinished();
	m_index = QModelIndex();
	m_sDefaultText.clear();
	m_pItemEdit->blockSignals(bBlockSignals);
}


// Shortcut text cancel notification.
void qtractorShortcutTableItemEditor::cancel (void)
{
	const bool bBlockSignals = m_pItemEdit->blockSignals(true);
	m_index = QModelIndex();
	m_sDefaultText.clear();
	emit editingFinished();
	m_pItemEdit->blockSignals(bBlockSignals);
}


//-------------------------------------------------------------------------
// qtractorShortcutTableItemDelegate

qtractorShortcutTableItemDelegate::qtractorShortcutTableItemDelegate (
	qtractorShortcutForm *pShortcutForm )
	: QItemDelegate(pShortcutForm->tableWidget()),
		m_pShortcutForm(pShortcutForm)
{
}


// Overridden paint method.
void qtractorShortcutTableItemDelegate::paint ( QPainter *pPainter,
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	// Special treatment for action icon+text...
	if (index.column() == 0) {
		QTableWidget *pTableWidget = m_pShortcutForm->tableWidget();
		QTableWidgetItem *pItem	= pTableWidget->item(index.row(), 0);
		pPainter->save();
		if (option.state & QStyle::State_Selected) {
			const QPalette& pal = option.palette;
			pPainter->fillRect(option.rect, pal.highlight().color());
			pPainter->setPen(pal.highlightedText().color());
		}
		// Draw the icon...
		QRect rect = option.rect;
		const QSize& iconSize = pTableWidget->iconSize();
		pPainter->drawPixmap(1,
			rect.top() + ((rect.height() - iconSize.height()) >> 1),
			pItem->icon().pixmap(iconSize));
		// Draw the text...
		rect.setLeft(iconSize.width() + 2);
		pPainter->drawText(rect,
			Qt::TextShowMnemonic | Qt::AlignLeft | Qt::AlignVCenter,
			pItem->text());
		pPainter->restore();
	} else {
		// Others do as default...
		QItemDelegate::paint(pPainter, option, index);
	}
}


QWidget *qtractorShortcutTableItemDelegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	// Keyboard shortcut.
	qtractorShortcutTableItemEditor *pItemEditor
		= new qtractorShortcutTableItemEditor(pParent);
	pItemEditor->setIndex(index);
	pItemEditor->setDefaultText(
		index.model()->data(index, Qt::DisplayRole).toString());
	QObject::connect(pItemEditor,
		SIGNAL(editingFinished()),
		SLOT(commitEditor()));
	return pItemEditor;
}


void qtractorShortcutTableItemDelegate::setEditorData ( QWidget *pEditor,
	const QModelIndex& index ) const
{
	qtractorShortcutTableItemEditor *pItemEditor
		= qobject_cast<qtractorShortcutTableItemEditor *> (pEditor);
	pItemEditor->setText(
		index.model()->data(index, Qt::DisplayRole).toString());
}


void qtractorShortcutTableItemDelegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel, const QModelIndex& index ) const
{
	qtractorShortcutTableItemEditor *pItemEditor
		= qobject_cast<qtractorShortcutTableItemEditor *> (pEditor);
	pModel->setData(index, pItemEditor->text());
}


void qtractorShortcutTableItemDelegate::commitEditor (void)
{
	qtractorShortcutTableItemEditor *pItemEditor
		= qobject_cast<qtractorShortcutTableItemEditor *> (sender());

	if (m_pShortcutForm->commitEditor(pItemEditor))
		emit commitData(pItemEditor);

	emit closeEditor(pItemEditor);
}


//-------------------------------------------------------------------------
// qtractorShortcutForm

// Constructor.
qtractorShortcutForm::qtractorShortcutForm (
	const QList<QAction *>& actions, QWidget *pParent ) : QDialog(pParent)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Window modality (let plugin/tool windows rave around).
	QDialog::setWindowModality(Qt::ApplicationModal);

	m_iDirtyActionShortcuts = 0;
	m_iDirtyActionControl = 0;

	m_iActionControlRow = -1;

	m_ui.ShortcutTable->setIconSize(QSize(16, 16));
	m_ui.ShortcutTable->setItemDelegate(
		new qtractorShortcutTableItemDelegate(this));

	QHeaderView *pHorizontalHeader = m_ui.ShortcutTable->horizontalHeader();
	pHorizontalHeader->setStretchLastSection(true);
	pHorizontalHeader->setDefaultAlignment(Qt::AlignLeft);
	pHorizontalHeader->resizeSection(0, 180);
	pHorizontalHeader->resizeSection(1, 320);
//	pHorizontalHeader->hideSection(3);

	QHeaderView *pVerticalHeader = m_ui.ShortcutTable->verticalHeader();
	const int iRowHeight = m_ui.ShortcutTable->fontMetrics().height() + 4;
	pVerticalHeader->setDefaultSectionSize(iRowHeight);
	pVerticalHeader->hide();

	int iRow = 0;
	QListIterator<QAction *> iter(actions);
	while (iter.hasNext()) {
		QAction *pAction = iter.next();
		if (pAction->objectName().isEmpty())
			continue;
		m_ui.ShortcutTable->insertRow(iRow);
		const QString& sActionText
			= qtractorActionControl::menuActionText(pAction, pAction->text());
		m_ui.ShortcutTable->setItem(iRow, 0,
			new qtractorShortcutTableItem(pAction->icon(), sActionText));
		m_ui.ShortcutTable->setItem(iRow, 1,
			new qtractorShortcutTableItem(pAction->statusTip()));
		const QKeySequence& shortcut = pAction->shortcut();
		const QString& sShortcutText = shortcut.toString();
		m_ui.ShortcutTable->setItem(iRow, 2,
			new qtractorShortcutTableItem(shortcut));
		m_ui.ShortcutTable->setItem(iRow, 3,
			new qtractorShortcutTableItem(actionControlText(pAction)));
		m_actions.insert(iRow, pAction);
		if (!sShortcutText.isEmpty())
			m_shortcuts.insert(sShortcutText, iRow);
		++iRow;
	}

	// Custom context menu...
	m_ui.ShortcutTable->setContextMenuPolicy(Qt::CustomContextMenu);

	// Restore last seen form position and extents...
	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions)
		pOptions->loadWidgetGeometry(this, true);

	QObject::connect(m_ui.ShortcutTable,
		SIGNAL(itemDoubleClicked(QTableWidgetItem *)),
		SLOT(actionShortcutActivated(QTableWidgetItem *)));

	QObject::connect(m_ui.ShortcutTable,
		SIGNAL(itemChanged(QTableWidgetItem *)),
		SLOT(actionShortcutChanged(QTableWidgetItem *)));

	QObject::connect(m_ui.ShortcutTable,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(actionControlMenuRequested(const QPoint&)));

	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	stabilizeForm();
}


qtractorShortcutForm::~qtractorShortcutForm (void)
{
	// Store form position and extents...
	qtractorOptions *pOptions = qtractorOptions::getInstance();
	if (pOptions)
		pOptions->saveWidgetGeometry(this, true);
}


// Action shortcut/control table widget accessor.
QTableWidget *qtractorShortcutForm::tableWidget (void) const
{
	return m_ui.ShortcutTable;
}


// Action shortcut/control dirty-flag accessors.
bool qtractorShortcutForm::isDirtyActionShortcuts (void) const
{
	return (m_iDirtyActionShortcuts > 0);
}

bool qtractorShortcutForm::isDirtyActionControl (void) const
{
	return (m_iDirtyActionControl > 0);
}


// Shortcut action finder & settler.
bool qtractorShortcutForm::commitEditor (
	qtractorShortcutTableItemEditor *pItemEditor )
{
	const QModelIndex& index = pItemEditor->index();
	if (!index.isValid())
		return false;

	const QString& sShortcutText = pItemEditor->text();
	const QString& sDefaultText = pItemEditor->defaultText();

	if (sShortcutText == sDefaultText)
		return false;

	if (!sShortcutText.isEmpty()) {
		if (m_shortcuts.contains(sShortcutText)) {
			QMessageBox::warning(this,
				tr("Warning") + " - " QTRACTOR_TITLE,
				tr("Keyboard shortcut (%1) already assigned.")
					.arg(sShortcutText),
				QMessageBox::Cancel);
			pItemEditor->clear();
			return false;
		}
		m_shortcuts.insert(sShortcutText, index.row());
	}

	if (!sDefaultText.isEmpty())
		m_shortcuts.remove(sDefaultText);

	return true;
}


void qtractorShortcutForm::actionShortcutActivated ( QTableWidgetItem *pItem )
{
	switch (pItem->column()) {
	case 3:
		// MIDI Controller...
		actionControlActivated();
		break;
	case 2:
	default:
		// Keyboard shortcut...
		m_ui.ShortcutTable->editItem(m_ui.ShortcutTable->item(pItem->row(), 2));
		break;
	}
}


void qtractorShortcutForm::actionShortcutChanged ( QTableWidgetItem *pItem )
{
	const QString& sShortcutText
		= QKeySequence(pItem->text().trimmed()).toString();
	pItem->setText(sShortcutText);
	++m_iDirtyActionShortcuts;

	stabilizeForm();
}


void qtractorShortcutForm::accept (void)
{
	if (m_iDirtyActionShortcuts > 0) {
		QHash<int, QAction *>::ConstIterator iter = m_actions.constBegin();
		const QHash<int, QAction *>::ConstIterator& iter_end = m_actions.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const QString& sShortcutText
				= m_ui.ShortcutTable->item(iter.key(), 2)->text();
			iter.value()->setShortcut(QKeySequence(sShortcutText));
		}
	}

	QDialog::accept();
}


void qtractorShortcutForm::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyActionShortcuts > 0) {
		QMessageBox::StandardButtons buttons
			= QMessageBox::Discard | QMessageBox::Cancel;
		if (m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->isEnabled())
			buttons |= QMessageBox::Apply;
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("Keyboard shortcuts have been changed.\n\n"
			"Do you want to apply the changes?"),
			buttons)) {
		case QMessageBox::Apply:
			accept();
			return;
		case QMessageBox::Discard:
			break;
		default:    // Cancel.
			bReject = false;
		}
	}
	else
	if (m_iDirtyActionControl > 0) {
		QMessageBox::StandardButtons buttons
			= QMessageBox::Discard | QMessageBox::Cancel;
		if (m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->isEnabled())
			buttons |= QMessageBox::Apply;
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " QTRACTOR_TITLE,
			tr("MIDI Controller shortcuts have been changed.\n\n"
			"Do you want to apply the changes?"),
			buttons)) {
		case QMessageBox::Apply:
			accept();
			return;
		case QMessageBox::Discard:
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


void qtractorShortcutForm::stabilizeForm (void)
{
	const bool bValid
		= (m_iDirtyActionShortcuts > 0 || m_iDirtyActionControl > 0);
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


void qtractorShortcutForm::actionControlMenuRequested ( const QPoint& pos )
{
	QMenu menu(this);

	menu.addAction(
		QIcon(":/images/itemControllers.png"),
		tr("&MIDI Controller..."), this,
		SLOT(actionControlActivated()));

	menu.exec(m_ui.ShortcutTable->viewport()->mapToGlobal(pos));
}


void qtractorShortcutForm::actionControlActivated (void)
{
	m_iActionControlRow = m_ui.ShortcutTable->currentRow();
	if (m_iActionControlRow < 0)
		return;

	QAction *pMidiObserverAction = m_actions.value(m_iActionControlRow, NULL);
	if (pMidiObserverAction == NULL)
		return;

	qtractorMidiControlObserverForm::showInstance(pMidiObserverAction, this);

	qtractorMidiControlObserverForm *pMidiObserverForm
		= qtractorMidiControlObserverForm::getInstance();
	if (pMidiObserverForm) {
		QObject::connect(pMidiObserverForm,
			SIGNAL(accepted()),
			SLOT(actionControlAccepted()));
	}
}


void qtractorShortcutForm::actionControlAccepted (void)
{
	if (m_iActionControlRow >= 0) {
		m_ui.ShortcutTable->setUpdatesEnabled(false);
		QAction *pMidiObserverAction
			= m_actions.value(m_iActionControlRow, NULL);
		QTableWidgetItem *pItem
			= m_ui.ShortcutTable->item(m_iActionControlRow, 3);
		if (pItem && pMidiObserverAction)
			pItem->setText(actionControlText(pMidiObserverAction));
		m_iActionControlRow = -1;
		m_ui.ShortcutTable->setUpdatesEnabled(true);
	}

	++m_iDirtyActionControl;
	stabilizeForm();
}


QString qtractorShortcutForm::actionControlText ( QAction *pAction ) const
{
	QString sActionControlText;

	qtractorActionControl *pActionControl
		= qtractorActionControl::getInstance();
	if (pActionControl) {
		qtractorActionControl::MidiObserver *pMidiObserver
			= pActionControl->getMidiObserver(pAction);
		if (pMidiObserver) {
			QStringList clist;
			clist.append(qtractorMidiControl::nameFromType(pMidiObserver->type()));
			clist.append(QString::number(pMidiObserver->channel() + 1));
			clist.append(QString::number(pMidiObserver->param()));
			sActionControlText = clist.join(", ");
		}
	}

	return sActionControlText;
}


// end of qtractorShortcutForm.cpp
