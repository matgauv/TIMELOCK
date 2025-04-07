# TIMELOCK : team-03

## M4 Required Elements:
<table>
  <tr>
   <td><strong># / Task</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Implemented Where?</strong>
   </td>
   <td><strong>Notes</strong>
   </td>
  </tr>
  <tr>
   <td>[1] 10-minutes of unique gameplay
   </td>
   <td>Playability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[2] Stability
   </td>
   <td>Stability
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[3] User Experience 
   </td>
   <td>Robustness
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[4] Robustness
   </td>
   <td>Robustness
   </td>
   <td><code>Whole application</code>
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>[5] Bug List
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/spreadsheets/d/14reigVHCiUrnIVMnTQdUnz6nX_JFBuTTT_pZXGZ06WA/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[6] Test plan - a list of player or game actions and their expected outcomes.
   </td>
   <td>Software Engineering
   </td>
   <td>
   </td>
   <td>
<code>https://docs.google.com/document/d/1hXkuZfs6yKLm8KzTLnPzLKjcXIdfgNQv9B_2E65kXTw/edit?usp=sharing</code>
   </td>
  </tr>
  <tr>
   <td>[7] Video
   </td>
   <td>Reporting
   </td>
   <td>
   </td>
   <td>
<code>https://youtu.be/kL9yFRfgWlU</code>
   </td>
  </tr>
</table>



# M3 Creative Elements

** these are the creative elements we wish to have graded for this milestone

<table>
  <tr>
   <td><strong>Element</strong>
   </td>
   <td><strong>Category</strong>
   </td>
   <td><strong>Level</strong>
   </td>
   <td><strong>Implemented Where? </strong>
   </td>
   <td><strong>Description</strong>
   </td>
  </tr>
  <tr>
   <td>[10] Precise Collisions
   </td>
   <td>Physics & Simulation
   </td>
   <td>Advanced
   </td>
   <td>
   </td>
   <td></td>
  </tr>
  <tr>
   <td>[27] Story elements
   </td>
   <td>Quality & User Experience 
   </td>
   <td>Basic
   </td>
   <td>
   </td>
   <td>Narrative driven cut-scenes for intro and outro
   </td>
  </tr>
</table>



## Other Notes:
- DEBUG CONTROLS: `N` will enter `no-clip` mode, `M` will enter fly mode (use arrow keys to fly). Use for exploring the level when needed, make sure to disable both before playing again!
- Background music source: https://www.youtube.com/watch?v=gfnQIaFBKIE
- Slowdown & speedup sound effect source: https://www.youtube.com/watch?v=z_epWpdsHJc
- How to run on linux/mac (and compile with debug symbols):
  - Navigate to `ECS_DRAFT/build`.
  - Run `cmake ..` (or `cmake -DCMAKE_BUILD_TYPE=Debug ..` for debug symbols)
  - Run `make` to compile changes
  - Run `./TIMELOCK`

- How to build on Windows Visual Studio:
  - Open folder `ECS_DRAFT`, wait for CMakeList.txt file to be loaded
  - Can also manually configure CMake cache by going to `Navigation bar` -> `Project` -> `Configure Cache`
  - Update CMake settings according to need; we suggest build and run release version of the game
    - Windows user can go to `/ECS_DRAFT/CMakeSettings.json` and add build modes, such as `x64-Release`
  - Build `TIMELOCK.exe`
  - Run `TIMELOCK.exe`
 
- How to use co author script:
    - make sure it is executable (`chmod +x ./coAuthorCommit.sh`)
    - Add all the files to git (`git add -p`)
    - Run the commit script to make the commit `./coAuthorCommit.sh "NAME" "EMAIL" "COMMIT MESSAGE"`
    - Push (`git push`)
