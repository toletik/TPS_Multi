[ProjectName] : TPSMulti
///////////////////////////////
 [Authors] : 
	Anatole TODOROV
	Jeremy Grondin
///////////////////////////////
 [Techs] : 
	C++
	UE 4
///////////////////////////////
 [To Launch] : 
	Launch at least one dedicated server (Launch the .bat at the root, if your UE4 installation is not by default you need to change the path in the .bat)
	Launch as many build player as wanted
///////////////////////////////
 [Inputs] : 
	WASD : Move Character
	Mouse Mvt : Rotate Camera
	Shift : Sprint
	E : Interact with Button
	R : Reload weapon
	F : Attack melee
	Tab : Display team info
	Mouse Left Click : Shoot
	Mouse Right Click : Aim
///////////////////////////////
 [Features] :	
	- Replication of all Players actions and animation/FX
	- Possibility to join the session of a dedicated server
	- Customization (name, team) in lobby before launch game (seamless travel) when everybody is ready
	- Many UI in game (minimap, overall team's score, detailed information about teams and their players, kill log at top right corner)
///////////////////////////////
 [Additionnal Notes] :
	- Sometimes the join session doesn't work
	- It can rarely happen that a player doesn't have working inputs
	- since last day we can't find dedicated server launched on another computer
	- The shoot is replicated but it happens that an AI can die on only one instance and still be alive for others players

