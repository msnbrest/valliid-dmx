<h2>Usage</h2>

# [ 25,  24, 0, 0, 5,  0, 1, 0, 2,  24, 0, 4, 24,  0, 0 ]
<p>this is a trame send in serial port, on base 26
it set DMX trame to [ 128, 5, 5 ]</p>

<h2>Details</h2>

"25" is "init" cmd
"624" is "edit" cmd (future is value, after is channels)
limits to value are "0 0" so 0, to "9 21" so 255.
liimts to channel are "0 0" so 0, to "19 17" so 511.

<h2>Exemples</h2>

exemple send 5 to DMX channels 2 & 3, and value 128 to DMX channel 1 :
(init, edit,value,   chan, chan,   edit,value,   chan )
# [ 25,  24, 0, 0, 5,  0, 1, 0, 2,  24, 0, 4, 24,  0, 0 ]

exemple send 7 to DMX channel 20 :
(init, edit,value,   chan )
# [ 25,  24, 0, 0, 7,  0, 20 ]

exemple send 29 to DMX channels 6 7 8 9 10 11 12 13 14 15 :
(init, edit,value,   chans... )
# [ 25,  24, 0, 1, 3,  0, 6, 0, 7, 0, 8, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 15 ]
