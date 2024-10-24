.global StartCalculateCounterDamage
.global BranchBackCalculateCounterDamage
.global StartCheckExplosiveKO
.global ConditionalBranchCheckExplosiveKO
.global BranchBackCheckExplosiveKO
.global StartToggleScopedAndCheckFreezeBreak
.global BranchBackToggleScopedAndCheckFreezeBreak
.global StartTrackPoisonDamage
.global BranchBackTrackPoisonDamage


StartCheckExplosiveKO:
lwz %r3, 0x4 (%r27)
cmpwi %r3, 0x12  # Hyper Bob-omb
beq- lbl_SkipBobombExplosiveKO
cmpwi %r3, 0x2b  # Bob-omb
bne+ lbl_DontSkipExplosiveKO

lbl_SkipBobombExplosiveKO:
ConditionalBranchCheckExplosiveKO:
b 0

lbl_DontSkipExplosiveKO:
# Restore original opcode, continue execution.
lwz %r3, 0x1b0 (%r28)
BranchBackCheckExplosiveKO:
b 0


StartCalculateCounterDamage:
# Counterattack work (input to BattleCheckCounter).
addi %r3, %r1, 0x54
# Original attacker.
mr %r4, %r31
# Original target + targeted part.
mr %r5, %r27
mr %r6, %r28
# Damage dealt to other party.
mr %r7, %r22
bl calculateCounterDamage

BranchBackCalculateCounterDamage:
b 0


StartToggleScopedAndCheckFreezeBreak:
# If this is the final hit in the attack...
lwz %r0, 0x10 (%r1)
rlwinm. %r0, %r0, 0, 0x100
beq- lbl_CheckFreezeBreak
# Toggle off Scoped status if a player attempted a harmful attack.
mr %r3, %r31
mr %r4, %r27
mr %r5, %r29
bl toggleOffScopedStatus

lbl_CheckFreezeBreak:
# Check for ice element, and skip new code if present.
lbz %r0, 0x6c (%r29)
cmpwi %r0, 2
beq- lbl_SkipFreezeBreak
# Check whether the attack can inflict Freeze, and skip new code if so.
lbz %r0, 0x92 (%r29)
cmpwi %r0, 0
bgt- lbl_SkipFreezeBreak
# Check for Freeze status on the target.
lbz %r0, 0x122 (%r27)
cmpwi %r0, 0
ble+ lbl_SkipFreezeBreak
# Check that the hit will do damage.
cmpwi %r22, 0
ble- lbl_SkipFreezeBreak
# Apply "Freeze break" effect to next hit.
lwz %r0, 0x2c (%r1)
ori %r0, %r0, 0x8000
stw %r0, 0x2c (%r1)
lbl_SkipFreezeBreak:
# Run original opcode.
lwz %r0, 0x10 (%r1)
BranchBackToggleScopedAndCheckFreezeBreak:
b 0


StartTrackPoisonDamage:
lwz %r10, 0xb40 (%r4)
add %r10, %r10, %r6
stw %r10, 0xb40 (%r4)
li %r10, 1  # Original opcode.
BranchBackTrackPoisonDamage:
b 0
