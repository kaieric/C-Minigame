# Minimalism:
This game is meant to be incredibly minimalist, for a minigame game jam. Fixed window size, no camera movement, no haptics or sound. Title scene and end scene are the same minimalist black screens with centered text stating click to begin.

# Inspiration
This game takes from tile based puzzle/economy games like Kingdom of Land and Houses on Itch.io. These have a set of tiles, each of which has properties and the player can interact with- typically building buildings on them.

# Graphics
16 or 32 bit tiles, importing images from the directory
transluscent scanlines, which make pixels under them a shade darker, like in balatro

# Test Version
Tileset of 5x5, start with a tile initializer, so I can deterministically make the board, and make a framework for adding in tiletypes. To start with, just have blank 16 bit tiles of a single color.

# Developer Tools
Optional: create some kind of tool that makes it easy to build larger maps of tiles- interacted with only by the developer, not by gamers.

# Export
Must be exportable to WebGL


# Game mechanics:
Three tile types: forests, plains, and mountains.
* Mountains are impassable, uninteractable tiles- equivalent to blank spaces.
* Plains can have buidings built on them (buildings should be something contained in a tile, rather than a tiletype themselves, one buiding per tile max).
* Forests are the main focus of the game- they store a certain amount of wood, and passively regenerate this. If a forest reaches 0 or negative regen rate from pollution, sprite change to gray. Once reaching 0 lumber in this state, the first disappears.
Two Buildings:
* Sawmills to harvest the lumber passively. The lumber collects in the sawmill.
* Factories take lumber from adjacent sawmill, and turn it into `capital`. Capital is used to build more buildings. Factories also produce pollution, which is a variable stored in a tile, and high pollution tiles spread to low pollution tiles. Pollution also reduces forest regen rate, and can turn it negative. Pollution represents the contamination motif of the game, cannot be done away with. Should also boost factory and sawmill workspeed.
Victory Condition:
* a variable representing all liquid capital, as well as the cost of all buildings built combined represents the total capital of the player. Reach a certain number of capital to win, and end the game.
Motif:
This is being made for a game jam as a minigame about contamination. The theme should be the factory creating and ultimately destroying the means of its own reproduction, with a slow start, explosive growth, and then collapse when the environment gives out.