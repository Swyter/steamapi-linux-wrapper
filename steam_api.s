.section .text
.globl SteamAPI_Init
.type SteamAPI_Init, @function
SteamAPI_Init:
	#jmp 0x7e7c9a8d
	#jmp   0x806573
	#jmp 0xfd799a8d + 0x2060000

	jmp 0x7e7c9a8d + 0x81030000


.globl SteamUser
.type SteamUser, @function
SteamUser:
	jmp 0x7e7c7f1b + 0x81030000
