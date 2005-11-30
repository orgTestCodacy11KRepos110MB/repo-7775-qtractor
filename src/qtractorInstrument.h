// qtractorInstrument.h
//
/****************************************************************************
   Copyright (C) 2005, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#ifndef __qtractorInstrument_h
#define __qtractorInstrument_h

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>


// Forward declarations.
class QTextStream;


//----------------------------------------------------------------------
// class qtractorInstrumentData -- instrument definition data classes.
//

class qtractorInstrumentData
{
public:

	typedef QMap<int, QString> DataMap;

	// Constructor.
	qtractorInstrumentData()
		: m_pData(new DataRef()) {}
	// Copy constructor.
	qtractorInstrumentData(const qtractorInstrumentData& data)
		{ attach(data); }
	// Destructor.
	~qtractorInstrumentData()
		{ detach(); }

	// Assignment operator.
	qtractorInstrumentData& operator= (const qtractorInstrumentData& data)
	{
		if (m_pData != data.m_pData) {
			detach();
			attach(data);
		}
		return *this;
	}

	// Accessor operator.
	QString& operator[] (int iIndex) { return m_pData->map[iIndex]; }

	// Property accessors.
	void setName(const QString& sName)
		{ m_pData->name = sName; }
	const QString& name() const { return m_pData->name; }

	void setBasedOn(const QString& sBasedOn)
		{ m_pData->basedOn = sBasedOn; }
	const QString& basedOn() const { return m_pData->basedOn; }

	// Indirect iterator stuff.
	typedef DataMap::Iterator Iterator;
	Iterator begin() { return m_pData->map.begin(); }
	Iterator end()   { return m_pData->map.end();}

	unsigned int count() const { return m_pData->map.count(); }

	bool contains(int iKey) const
		{ return m_pData->map.contains(iKey); }

protected:

	// Copy/clone method.
	void attach(const qtractorInstrumentData& data)
		{  m_pData = data.m_pData; m_pData->refCount++; }
	// Destroy method.
	void detach()
		{ if (--(m_pData->refCount) == 0) delete m_pData; }

private:

	// The ref-counted data.
	struct DataRef
	{
		// Default payload constructor.
		DataRef() : refCount(1) {};
		// Payload members.
		int     refCount;
		QString name;
		QString basedOn;
		DataMap map;

	} * m_pData;
};

class qtractorInstrumentDataList
	: public QMap<QString, qtractorInstrumentData> {};

class qtractorInstrumentPatches
	: public QMap<int, qtractorInstrumentData> {};

class qtractorInstrumentNotes
	: public QMap<int, qtractorInstrumentData> {};

class qtractorInstrumentKeys
	: public QMap<int, qtractorInstrumentNotes> {};

class qtractorInstrumentDrumFlags
	: public QMap<int, int> {};

class qtractorInstrumentDrums
	: public QMap<int, qtractorInstrumentDrumFlags> {};


//----------------------------------------------------------------------
// class qtractorInstrument -- instrument definition instance class.
//

class qtractorInstrument
{
public:

	// Constructor.
	qtractorInstrument()
		: m_pData(new DataRef()) {}
	// Copy constructor.
	qtractorInstrument(const qtractorInstrument& instr)
		{ attach(instr); }
	// Destructor.
	~qtractorInstrument()
		{ detach(); }

	// Assignment operator.
	qtractorInstrument& operator= (const qtractorInstrument& instr)
	{
		if (m_pData != instr.m_pData) {
			detach();
			attach(instr);
		}
		return *this;
	}

	// Instrument title property accessors.
	void setInstrumentName(const QString& sInstrumentName)
		{ m_pData->instrumentName = sInstrumentName; }
	const QString& instrumentName() const { return m_pData->instrumentName; }

	// BankSelMethod accessors.
	void setBankSelMethod(int iBankSelMethod)
		{ m_pData->bankSelMethod = iBankSelMethod; }
	int bankSelMethod() const { return m_pData->bankSelMethod; }

	void setUsesNotesAsControllers(bool bUsesNotesAsControllers)
		{ m_pData->usesNotesAsControllers = bUsesNotesAsControllers; }
	bool usesNotesAsControllers() const
		{ return m_pData->usesNotesAsControllers; }

	// Patch banks accessors.
	qtractorInstrumentPatches& patches() { return m_pData->patches; }
	qtractorInstrumentData& patch(int iBank) const;
	void setPatch(int iBank, const qtractorInstrumentData& patch)
		{ m_pData->patches[iBank] = patch; }

	// Control names accessors.
	void setControlName(const QString& sControlName)
		{ m_pData->control.setName(sControlName); }
	const QString& controlName() const { return m_pData->control.name(); }
	qtractorInstrumentData& control() const { return m_pData->control; }
	void setControl(const qtractorInstrumentData& control)
		{ m_pData->control = control; }

	// RPN names accessors.
	void setRpnName(const QString& sRpnName)
		{ m_pData->rpn.setName(sRpnName); }
	const QString& rpnName() const { return m_pData->rpn.name(); }
	qtractorInstrumentData& rpn() const { return m_pData->rpn; }
	void setRpn(const qtractorInstrumentData& rpn)
		{ m_pData->rpn = rpn; }

	// NRPN names accessors.
	void setNrpnName(const QString& sNrpnName)
		{ m_pData->nrpn.setName(sNrpnName); }
	const QString& nrpnName() const { return m_pData->nrpn.name(); }
	qtractorInstrumentData& nrpn() const { return m_pData->nrpn; }
	void setNrpn(const qtractorInstrumentData& nrpn)
		{ m_pData->nrpn = nrpn; }

	// Keys banks accessors.
	qtractorInstrumentKeys& keys() { return m_pData->keys; }
	qtractorInstrumentData& notes(int iBank, int iProg) const;
	void setNotes(int iBank, int iProg, const qtractorInstrumentData& notes)
		{ m_pData->keys[iBank][iProg] = notes; }

	// Drumflags banks accessors.
	qtractorInstrumentDrums& drums() { return m_pData->drums; }
	bool isDrum(int iBank, int iProg) const;
	void setDrum(int iBank, int iProg, bool bDrum)
		{ m_pData->drums[iBank][iProg] = (int) bDrum; }

protected:

	// Copy/clone method.
	void attach(const qtractorInstrument& instr)
		{  m_pData = instr.m_pData; m_pData->refCount++; }
	// Destroy method.
	void detach()
		{ if (--(m_pData->refCount) == 0) delete m_pData; }

private:

	// The ref-counted data.
	struct DataRef
	{
		// Default payload constructor.
		DataRef() : refCount(1),
			bankSelMethod(0), usesNotesAsControllers(false) {};
		// Payload members.
		int                       refCount;
		int                       bankSelMethod;
		bool                      usesNotesAsControllers;
		QString                   instrumentName;
		qtractorInstrumentPatches patches;
		qtractorInstrumentData    control;
		qtractorInstrumentData    rpn;
		qtractorInstrumentData    nrpn;
		qtractorInstrumentKeys    keys;
		qtractorInstrumentDrums   drums;

	} * m_pData;
};


//----------------------------------------------------------------------
// class qtractorInstrumentList -- A Cakewalk .ins file container class.
//

class qtractorInstrumentList : public QMap<QString, qtractorInstrument>
{
public:

	// Open file methods.
	bool load(const QString& sFilename);
	bool save(const QString& sFilename);

	// The official loaded file list.
	const QStringList& files() const;

	// Manage a file list (out of sync)
	void appendFile(const QString& sFilename)
	    { m_files.append(sFilename); }
	void removeFile(const QString& sFilename)
	    { m_files.remove(sFilename); }

	// Patch Names definition accessors.
	qtractorInstrumentDataList& patches()
		{ return m_patches; }
	qtractorInstrumentData& patch(const QString& sName)
		{ return m_patches[sName]; }

	// Note Names definition accessors.
	qtractorInstrumentDataList& notes()
		{ return m_notes; }
	qtractorInstrumentData& note(const QString& sName)
		{ return m_notes[sName]; }

	// Controller Names definition accessors.
	qtractorInstrumentDataList& controllers()
		{ return m_controllers; }
	qtractorInstrumentData& controller(const QString& sName)
		{ return m_controllers[sName]; }

	// RPN Names definition accessors.
	qtractorInstrumentDataList& rpns()
		{ return m_rpns; }
	qtractorInstrumentData& rpn(const QString& sName)
		{ return m_rpns[sName]; }

	// NRPN Names definition accessors.
	qtractorInstrumentDataList& nrpns()
		{ return m_nrpns; }
	qtractorInstrumentData& nrpn(const QString& sName)
		{ return m_nrpns[sName]; }

	// Clear all contents.
	void clearAll();

	// Special instrument list merge method.
	void merge(qtractorInstrumentList& instruments);

protected:

	// Internal instrument data list save method helpers.
	void saveDataList(QTextStream& ts, qtractorInstrumentDataList& list);
	void saveData(QTextStream& ts, qtractorInstrumentData& data);

	// Special instrument data list merge method.
	void mergeDataList(qtractorInstrumentDataList& dst,
		qtractorInstrumentDataList& src);

private:

	// To hold the names definition lists.
	qtractorInstrumentDataList m_patches;
	qtractorInstrumentDataList m_notes;
	qtractorInstrumentDataList m_controllers;
	qtractorInstrumentDataList m_rpns;
	qtractorInstrumentDataList m_nrpns;
	
	// To old the official file list.
	QStringList m_files;
};


#endif  // __qtractorInstrument_h


// end of qtractorInstrument.h
