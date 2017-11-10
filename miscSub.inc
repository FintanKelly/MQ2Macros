|**
	Random Useful Subroutines
	(Harmtouch/Illiad)
**|

|**
	This subroutine takes the npc ID, any loot options, and then kills it.
**|
Sub killMob(int mobID, conColor, killOption, lootOption)

	| Local Variable
	/declare killTimeOut timer
	
	| Do we actually want to kill it?
	/if ((${mobID} != ${Me.ID}) && ((${Spawn[${mobID}].CleanName.Equal[${killOption}]}) || (${killOption.Equal[All]})) && (${Ini[varList.ini, mobList, ${Spawn[${mobID}].CleanName},Negative].NotEqual[Skip]}) && ((${Spawn[${mobID}].ConColor.Find[${conColor}]}) || ${conColor.Equal[All]}) && (${Target.Z} > -50)) {
		/target id ${mobID}
		
		:Reposition
			/if ((${Target.Distance} > 20) || (!${Target.LineOfSight})) {
				/warp loc ${Math.Calc[${Target.Y} - 7]} ${Math.Calc[${Target.X} - 7]} ${Math.Calc[${Target.Z} - ${Target.Height}/2]}
			}
		| End of :Reposition
		
		| Start killing!
		/delay 1s (${Target.Distance} < 10)
		/attack on
		/stick 10 moveback
		/delay 2
		/pet attack
		
		/varset killTimeOut 5s
		
		:DistanceCheck
		/if ((${Target.Distance} > 20) || (!${Target.LineOfSight})) {
			/warp loc ${Math.Calc[${Target.Y}]} ${Math.Calc[${Target.X}]} ${Target.Z}
		}
			:KillMob
				| Are we bugged out?
				/if (!${killTimeOut} && (${Target.PctHPs} >= 94)) {
					/return
				}
				
				/if (${Target.Type.NotEqual[CORPSE]}) {
					/if (${Me.Ducking}) {
						/goto :Reposition
					}
					
					/face fast nolook
						
					/call useAbility 
					
					/if (${Target.Type.NotEqual[CORPSE]}) {
						/goto :Reposition
					}
				}
			| End of :KillMob
		| End of :DistanceCheck
	}
	
	|**
	| Loot up!
	/if (${Spawn[${mobID}].Type.Equal[CORPSE]}) {
		/if (${lootOption.Equal[All]}) {
			/call lootMob ${mobID} "${lootOption}"
		} else /if (${Spawn[${mobID}].CleanName.Find[${lootOption}]}) {
			/call lootMob ${mobID} "${lootOption}"
		}
	}
	**|

	| Loot all nearby bodies
	:LootNearby
		/if (${SpawnCount[corpse radius 100]} > 0) {
			/if ((${lootOption.Equal[All]}) || (${NearestSpawn[corpse radius 100].CleanName.Find[${lootOption}]})) {
				/call lootMob ${NearestSpawn[corpse radius 100].ID} "lootOption"
			}

			/goto :LootNearby
		}
	| End of :LootNearby

/return

|**
	This small subroutine takes the ID of an item slot on the mob's corpse,
	and then attempts to take the item.
**|
sub lootCursor(int lootSlot)

	| Local Variable List
	/declare itemName string ${Cursor.Name}
	
	/shift /itemnotify loot${lootSlot} leftmouseup
	
	/delay 1s

	:LootReal
		/if (${Select[${Ini[varList.ini, itemList, ${itemName},Negative]},Keep,Sell,Reclaim]}) {
			/autoinv
		} else /if (${Ini[varList.ini, itemList, ${itemName},Negative].Equal[Open]}){
			/itemnotify pack8 leftmouseup
			/delay 1s
			
			/call openPackages
		} else {
			/destroy
		}
		
		/delay 1s
		
		/if (${Cursor.ID}) {
			/goto :LootReal
		}
		
	| End of :LootReal

	/if (${Ini[varList.ini, itemList, ${itemName},Negative].Equal[Reclaim]}) {
		/call reclaimCurrency ${itemName}
	}
	
/return

|**
	This small subroutine takes the ID of an item slot on the mob's corpse,
	and then attempts to take the item.
**|
sub lootItem(int lootSlot)
	
	| Local Variable List
	/declare itemName string ${Corpse.Item[${lootSlot}].Name}
	
	/if (${Ini[varList.ini, itemList, ${itemName},Negative].Equal[Open]}) {
		/shift /itemnotify loot${lootSlot} leftmouseup
		/delay 1s
		/itemnotify pack8 leftmouseup
		/delay 1s
		
		/call openPackages
	}
	
	| Click on the item and loot!
	/shift /itemnotify loot${lootSlot} rightmouseup
	
	:LootItem
		/if (${Corpse.Item[${lootSlot}].ID}) {
			/delay 1s
			/goto :LootItem
		}
	| End of :LootItem
	
	/if (${Ini[varList.ini, itemList, ${itemName},Negative].Equal[Reclaim]}) {
		/call reclaimCurrency ${itemName}
	}
	
/return

|**
	This takes the npc ID of the mob's corpse that you want to loot,
	and then loots it.
**|
sub lootMob(int mobID, lootOption, lootValue)

	| Local Variables
	/declare lootTimeOut timer
	/declare lootCount int
	/declare lootCounter int
	/declare lootName string

	| Does the mob have loot? Do you have inventory space?
	
	/if (${Me.FreeInventory}) {
		/target id ${mobID}
		/stick 5
		/delay 2s (${Target.Distance} < 6)
		
		/loot
		
		| Open teh lootz!
		/delay 2
		:OpenWindow
			/if (!${Window[LootWnd].Open}) {
				/target id ${mobID}
				/delay 5
				/delay 1s ${Window[LootWnd].Open}
				/loot
				
				| Did we bug out and lose corpse?
				/if (${Spawn[${mobID}].Name.Length} != NULL || ${Target.DistanceZ} <= 10) {
					/loot
				} else {
					/return
				}
		
				/goto :OpenWindow
			}
		| End of :OpenWindow
		
		/delay 4
		
		/vardata lootCount Corpse.Items
			
		| Lootz!
		/if (${lootCount}) {
			/for lootCounter 1 to ${lootCount}
				/if (!${Me.FreeInventory}) {
					/return
				}

				/if (${Corpse.Item[${lootCounter}].ID}) {
					/varset lootName ${Corpse.Item[${lootCounter}].Name}
					
					/if (${Ini[varList.ini, zoneList, ${Zone.ShortName}].Length} == 0) {
						/ini "varList" "zoneList" "${Zone.ShortName}" "Regular"
					}
					
					/if (${Ini[varList.ini, zoneList, ${Zone.ShortName},Negative].Equal[Cursor]}) {
						/call lootCursor ${lootCounter}
					} else /if ((${lootOption.Find[All]}) || (${Select[${Ini[varList.ini, itemList, ${lootName},Negative]},Keep,Sell,Open,Reclaim,Cash]}) || (${Corpse.Item[${lootCounter}].Class[${Me.Class.Name}].Name.Length})) {
						/if ((${lootValue} < 0) || (${Corpse.Item[${lootCounter}].Value} >= ${lootValue}) || (${Corpse.Item[${lootCounter}].AC} >= 400) || (${Ini[varList.ini, itemList, ${lootName},Negative].Equal[Keep]})) {
							/call lootItem ${lootCounter}

							/if (!${Select[${Ini[varList.ini, itemList, ${lootName},Negative]},Keep,Sell,Open,Reclaim]}) {
								/ini "varList.ini" "itemList" "${lootName}" "Sell"
							}

							/if (${Ini[varList.ini, mobList, ${Spawn[${mobID}].CleanName},Negative].NotEqual[Cash]}) {
								/ini "varList.ini" "mobList" "${Spawn[${mobID}].CleanName}" "Cash"
								/echo Added ${Spawn[${mobID}].CleanName}
							}
						}
					}
				}
			/next lootCounter
		} else {
			/echo Mob Empty
		}
		
		| Close teh lootz!
		/notify LootWnd DoneButton leftmouseup
			
		:CloseWindow
			/if (${Window[LootWnd].Open}) {
				/delay 5
				/goto :CloseWindow
			}
		| End of :CloseWindow
	} else {
		/call sellItems
	}

/return

|**
	Run any pre-killing setup.
**|
Sub macroSetup

	| Account for fall damage
	/setgrav .2
	
	| Set Corse Loot Type
	/hidec looted
	
	/delay 5

/return

|**
	This will open each bag and check for an Armor Package to open.
**|
Sub openPackages

	/itemnotify pack8 rightmouseup
	/delay 1s
	
	:ClearCursor
		/autoinv
		
		/if (${Cursor.Name.Length}) {
			/goto :ClearCursor
		}
	| End of :ClearCursor
	
/return

|**
	This uses my priority killing algorithm
**|
Sub priorityKill(conColor, killOption, lootOption, zoneOption)

	| Variable List
	/declare spawnCounter int
	
	| Priority 1 - Immediate Mobs
	:ImmediateMobs
		| Kill the trash around you first
		/if ((${Me.XTarget[1].ID}) && (${Ini[varList.ini, mobList, ${Me.XTarget[1].Name},Negative].NotEqual[Skip]})) {
			/for spawnCounter 1 to ${Me.XTarget[]}
				/if (!${Zone.ShortName.Find[${zoneOption}]}) {
					/return
				}
				
				/call killMob ${Me.XTarget[1].ID} ${conColor} "${killOption}" "${lootOption}"
			/next spawnCounter
		}
	| End of :ImmediateMobs

	| Priority 2 - Named Mobs
	:NamedMobs
		| Kill all of the named in the zone
		/if (${SpawnCount[npc # targetable]} >= 1) {
			/for spawnCounter 1 to ${SpawnCount[npc # targetable]}
				/if (!${Zone.ShortName.Find[${zoneOption}]}) {
					/return
				}
				
				/call killMob ${NearestSpawn[${spawnCounter}, npc # targetable].ID} ${conColor} "${killOption}" "${lootOption}"
				
				/if (${Me.XTarget[1].ID} != 0) {
					/goto :ImmediateMobs
				}
			/next spawnCounter
		}
	| End of :NamedMobs
	
	| Priority 3 - Trash Mobs
	:TrashMobs
		| Cycle through all of the trash in the zone
		/if (${SpawnCount[npc targetable]} >= 1) {
			/for spawnCounter 1 to ${SpawnCount[npc targetable]}
				/if (!${Zone.ShortName.Find[${zoneOption}]}) {
					/return
				}
				
				/call killMob ${NearestSpawn[${spawnCounter}, npc targetable].ID} ${conColor} "${killOption}" "${lootOption}"
				
				/if (${Me.XTarget[1].ID} != 0) {
					/goto :ImmediateMobs
				}
			/next spawnCounter
		}
	| End of :TrashMobs

/return

|**
	This takes a currency name and then attempts to reclaim any in your inventory.
**|
Sub reclaimCurrency(currencyName)

	/declare itemIndex int

	/if (${Window[IW_AltCurrPage].Open}) {
		/varset itemIndex ${Window[IW_AltCurrPage].Child[IW_AltCurr_PointList].List[${currencyName},2]}
		
		/notify IW_AltCurrPage IW_AltCurr_PointList listselect ${itemIndex}
		/notify IW_AltCurrPage IW_AltCurr_PointList leftmouse ${itemIndex}
		/delay 3
		
		/notify IW_AltCurrPage IW_AltCurr_ReclaimButton leftmouseup
		/delay 5
	}
	
/return

|**
	This subroutine goes through the process of selling all unwanted items.
**|
sub sellItems(sellValue)

	| Variable List
	/declare invCounter int
	/declare bagCounter int
	/declare cleanUp int 0
	
	| Open all of the inventory bags
	/for invCounter 1 to 8
		/if (${Me.Inventory[pack${invCounter}].Container}) {
			:OpenBag
				/itemnotify pack${invCounter} rightmouseup
				/delay 5 ${Window[pack${invCounter}].Open}
				
				/if (!${Window[pack${invCounter}].Open}) {
					/goto :OpenBag
				}
			| End of :OpenBag
		}
	/next invCounter
	
	| Sell items!
	:Sell
		/for invCounter 1 to 8
			/if (${Me.Inventory[pack${invCounter}].Container}) {
				/for bagCounter 1 to ${Me.Inventory[pack${invCounter}].Container}					
					/if (${Ini[varList.ini, itemList, ${Me.Inventory[pack${invCounter}].Item[${bagCounter}].Name},Negative].Equal[Sell]} && (${Me.Inventory[pack${invCounter}].Item[${bagCounter}].Value} >= ${sellValue})) {
						/itemnotify in pack${invCounter} ${bagCounter} leftmouseup
						/delay 1
						/notify MerchantWnd MW_Sell_Button leftmouseup
						/delay 5
					}
					/delay 10 ${Window[QuantityWnd].Open} || !${Bool[${Me.Inventory[pack${invCounter}].Item[${bagCounter}]}]}
					/if (${Window[QuantityWnd].Open}) {
						/notify QuantityWnd QTYW_Accept_Button leftmouseup
						/delay 10 !${Window[QuantityWnd].Open}
					}
					/delay 8 !${Bool[${Me.Inventory[pack${invCounter}].Item[${bagCounter}]}]}
				/next bagCounter
			}
		/next invCounter
		
		| Loop again to catch missed items
		/if (!${cleanUp}) {
			/varset cleanUp 1
			/goto :Sell
		}
	| End of :Sell

/return

|**
	This will take an ability name and type, and then attempt to cast it if it's ready.
**|
Sub useAbility()
	
	| Variable List
	/declare abilCounter int
	/declare abilityName string
	/declare abilityType string
	/declare abilityDelay string
	
	/for abilCounter 1 to ${Math.Calc[${Ini[varList.ini, abilityList,,].Count[|]}-1]}
		/varset abilityName ${Ini[varList.ini, abilityList,,].Arg[${abilCounter},|]}
		/varset abilityType ${Ini[varList.ini, abilityList, ${abilityName}].Arg[1,|]}
		/varset abilityDelay ${Ini[varList.ini, abilityList, ${abilityName}].Arg[2,|]}

		/if ((${abilityType.Find[spell]}) && (${Cast.Ready[${abilityName}|gem${abilityType.Right[1]}]})) {
			/delay ${abilityDelay}
			
			/casting "${abilityName}" Gem${abilityType.Right[1]}
		} else /if ((${abilityType.Equal[disc]}) && (${Me.CombatAbilityReady[${abilityName}]})) {
			/delay ${abilityDelay}
			
			/disc "${abilityName}"
		} else /if ((${Select[${abilityType},item,alt]}) && (${Cast.Ready[${abilityName}|${abilityType}]})) {
			/delay ${abilityDelay}
			
			/casting "${abilityName}" ${abilityType}
		}
	/next abilCounter
	
	|/delay ${Math.Calc[${Cast.Timing} + 10]}
	/delay !${Window[CastingWindow].Open}

/return