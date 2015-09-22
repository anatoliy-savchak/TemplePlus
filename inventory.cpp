#include "stdafx.h"
#include "common.h"
#include "inventory.h"
#include "obj.h"
#include "critter.h"

InventorySystem inventory;

struct InventorySystemAddresses : temple::AddressTable
{
	int (__cdecl*ItemGetAdvanced)(objHndl item, objHndl parent, int slotIdx, int flags);
	int(__cdecl*GetParent)(objHndl, objHndl*);
	InventorySystemAddresses()
	{
		rebase(GetParent, 0x10063E80);
		rebase(ItemGetAdvanced, 0x1006A810);
	}
} addresses;

objHndl InventorySystem::FindItemByName(objHndl container, int nameId) {
	return _FindItemByName(container, nameId);
}

objHndl InventorySystem::FindItemByProtoHandle(objHndl container, objHndl protoHandle, bool skipWorn) {
	return _FindItemByProto(container, protoHandle, skipWorn);
}

objHndl InventorySystem::FindItemByProtoId(objHndl container, int protoId, bool skipWorn) {
	auto protoHandle = objects.GetProtoHandle(protoId);
	return _FindItemByProto(container, protoHandle, skipWorn);
}

int InventorySystem::SetItemParent(objHndl item, objHndl parent, int flags)  {
	return _SetItemParent(item, parent, flags);
}

int InventorySystem::IsNormalCrossbow(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		int weapType = objects.getInt32(weapon, obj_f_weapon_type);
		if (weapType == wt_heavy_crossbow || weapType == wt_light_crossbow)
			return 1; // TODO: should this include repeating crossbow? I think the context is reloading action in some cases
		// || weapType == wt_hand_crossbow
	}
	return 0;
}

int InventorySystem::IsThrowingWeapon(objHndl weapon)
{
	if (objects.GetType(weapon) == obj_t_weapon)
	{
		WeaponAmmoType ammoType = (WeaponAmmoType)objects.getInt32(weapon, obj_f_weapon_ammo_type);
		if (ammoType > wat_dagger && ammoType <= wat_bottle) // thrown weapons   TODO: should this include daggers??
		{
			return 1;
		}
	}
	return 0;
}

ArmorType InventorySystem::GetArmorType(int armorFlags)
{
	if (armorFlags & ARMOR_TYPE_NONE)
		return ARMOR_TYPE_NONE;
	return (ArmorType) (armorFlags & (ARMOR_TYPE_LIGHT | ARMOR_TYPE_MEDIUM | ARMOR_TYPE_HEAVY) );
}

int InventorySystem::ItemDrop(objHndl item)
{
	return _ItemDrop(item);
}

int InventorySystem::GetParent(objHndl item)
{
	objHndl parent = 0i64;
	if (!addresses.GetParent(item, &parent))
		return 0;
	return parent;
	
}

obj_f InventorySystem::GetInventoryListField(objHndl objHnd)
{
	if (objects.IsCritter(objHnd)) 	return obj_f_critter_inventory_list_idx;
	if (objects.IsContainer(objHnd)) return obj_f_container_inventory_list_idx;
	return (obj_f)0;
}

int InventorySystem::ItemRemove(objHndl item)
{
	return _ItemRemove(item);
}

int InventorySystem::ItemGetAdvanced(objHndl item, objHndl parent, int slotIdx, int flags)
{
	return addresses.ItemGetAdvanced(item, parent, slotIdx, flags);
}