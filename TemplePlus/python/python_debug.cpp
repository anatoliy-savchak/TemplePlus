
#include "stdafx.h"
#include "python_debug.h"
#include "python_object.h"
#include "../condition.h"
#include "../radialmenu.h"
#include "feat.h"
#include <ai.h>

/*
	Dumps all conditions from the global hashtable to a Wiki article.
*/
PyObject *PyDebug_DumpConds() {
	
	auto cap = conds.mCondStructHashtable->numItems;
	auto result = PyList_New(cap);

	for (size_t i = 0; i < cap; ++i) {
		auto idx = conds.mCondStructHashtable->idxArray[i];
		auto data = conds.mCondStructHashtable->dataArray[idx];
		
		auto hooks = PyList_New(0);
		auto hook = data->subDispDefs;
		while (hook->dispType) {
			auto v = Py_BuildValue("IIIII", hook->dispCallback, hook->data1, hook->data2, hook->dispType, hook->dispKey);
			PyList_Append(hooks, v);
			hook++;
		}

		auto c = Py_BuildValue("sIO", data->condName, data->numArgs, hooks);
		Py_DecRef(hooks);
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

static void SetDictItem(PyObject *dict, const char *keyName, PyObject *val) {
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

static void SetDictItem(PyObject *dict, const char *keyName, int value) {
	auto val = PyInt_FromLong(value);
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

static void SetDictItem(PyObject *dict, const char *keyName, const char *text) {
	auto val = PyString_FromString(text);
	PyDict_SetItemString(dict, keyName, val);
	Py_DECREF(val);
}

PyObject* DumpRadialNode(const RadialMenuNode &node) {
	auto result = PyDict_New();

	// Dump the entry
	auto entry = node.entry;	
	SetDictItem(result, "text", entry.text);
	SetDictItem(result, "field4", entry.field4);
	SetDictItem(result, "textHash", entry.textHash);
	SetDictItem(result, "fieldc", entry.fieldc);
	SetDictItem(result, "type", (int) entry.type);
	SetDictItem(result, "minArg", entry.minArg);
	SetDictItem(result, "maxArg", entry.maxArg);
	SetDictItem(result, "actualArg", entry.actualArg);
	SetDictItem(result, "d20ActionType", entry.d20ActionType);
	SetDictItem(result, "d20ActionData1", entry.d20ActionData1);
	SetDictItem(result, "d20Caf", entry.d20Caf);
	SetDictItem(result, "spellEnumOrg", entry.d20SpellData.spellEnumOrg);
	SetDictItem(result, "spellMetaMagic", entry.d20SpellData.metaMagicData.metaMagicFlags);
	SetDictItem(result, "dispKey", entry.dispKey);
	SetDictItem(result, "callback", (uint32_t) entry.callback);
	SetDictItem(result, "flags", (uint32_t) entry.flags);
	SetDictItem(result, "helpId", (uint32_t) entry.helpId);
	SetDictItem(result, "field44", (uint32_t) entry.field44);

	auto children = PyList_New(node.childCount);
	for (int i = 0; i < node.childCount; ++i) {
		PyList_SET_ITEM(children, i, PyInt_FromLong(node.children[i]));
	}
	SetDictItem(result, "children", children);
	SetDictItem(result, "morphsTo", PyInt_FromLong(node.morphsTo));
	SetDictItem(result, "parent", PyInt_FromLong(node.parent));

	return result;
}

PyObject *PyDebug_DumpRadial() {	
	auto result = PyList_New(0);

	for (auto &radial : radialMenus.GetAll()) {
		auto item = PyDict_New();

		// Set owner of the radial menu
		SetDictItem(item, "owner", PyObjHndl_Create(radial->obj));
		SetDictItem(item, "field8", PyInt_FromLong(radial->field8));

		auto nodes = PyList_New(radial->nodeCount);
		for (int i = 0; i < radial->nodeCount; ++i) {
			PyList_SET_ITEM(nodes, i, DumpRadialNode(radial->nodes[i]));
		}
		SetDictItem(item, "nodes", nodes);

		PyList_Append(result, item);
		Py_DECREF(item);
	}

	return result;
}

/*
	dumps feat data tables so they can finally be externalized!
*/
PyObject *PyDebug_DumpFeats() {
	auto cap = NUM_FEATS;
	auto result = PyList_New(cap);
	uint32_t * featPropertiesTable = feats.m_featPropertiesTable;
	FeatPrereqRow * featPreReqTable = feats.m_featPreReqTable;

	for (size_t i = 0; i < cap; ++i) {
		feat_enums feat = (feat_enums)i;
		char * featName = feats.GetFeatName(feat);

		auto featPrereqs = PyList_New(0);
		for (int j = 0; j < 8; j++)
		{
			auto v = Py_BuildValue("II", featPreReqTable[i].featPrereqs[j].featPrereqCode, 
				featPreReqTable[i].featPrereqs[j].featPrereqCodeArg);
			PyList_Append(featPrereqs, v);
		}

		auto c = Py_BuildValue("sIIO", featName, feat, featPropertiesTable[i], featPrereqs);
		Py_DecRef(featPrereqs);
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

PyObject *PyDebug_DumpD20Actions() {
	auto cap = D20A_NUMACTIONS;
	auto result = PyList_New(cap);
	D20ActionDef * d20Defs = d20Sys.d20Defs;
	
	for (size_t i = 0; i < cap; ++i) {

		auto c = Py_BuildValue("IIIIIIIIIIIII", i, 
			d20Defs[i].addToSeqFunc, d20Defs[i].aiCheckMaybe, d20Defs[i].actionCheckFunc,
			d20Defs[i].tgtCheckFunc, d20Defs[i].locCheckFunc, d20Defs[i].performFunc,
			d20Defs[i].actionFrameFunc, d20Defs[i].projectilePerformFunc, d20Defs[i].pad_apparently,
			d20Defs[i].actionCost, d20Defs[i].pickerFuncMaybe, d20Defs[i].flags);
		
		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

PyObject *PyDebug_DumpAiTactics() {
	auto cap = 44;
	auto result = PyList_New(cap);
	AiTacticDef * aiTacDefs = aiSys.aiTacticDefs;

	for (size_t i = 0; i < cap; ++i) {

		auto c = Py_BuildValue("sII", 
			aiTacDefs[i].name, aiTacDefs[i].aiFunc, aiTacDefs[i].onInitiativeAdd );

		PyList_SET_ITEM(result, i, c);
	}

	return result;
}

static PyMethodDef PyDebug_Methods[] = {
	{ "dump_conds", (PyCFunction) PyDebug_DumpConds, METH_NOARGS, NULL },
	{ "dump_radial", (PyCFunction) PyDebug_DumpRadial, METH_NOARGS, NULL },
	{ "dump_feats", (PyCFunction)PyDebug_DumpFeats, METH_NOARGS, NULL },
	{ "dump_d20actions", (PyCFunction)PyDebug_DumpD20Actions, METH_NOARGS, NULL },
	{ "dump_ai_tactics", (PyCFunction)PyDebug_DumpAiTactics, METH_NOARGS, NULL },
	{ NULL, }
};

void PyDebug_Init() {
	Py_InitModule("debug", PyDebug_Methods);
}