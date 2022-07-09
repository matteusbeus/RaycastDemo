        .text

        .align  2
fileName1:
        .asciz  "/images/brickskull.dci3"
fileName2:
        .asciz  "/images/rock.dci3"
fileName3:
        .asciz  "/images/rockskull.dci3"
fileName4:
        .asciz  "/images/brick.dci3"
fileName5:
        .asciz  "/images/pistol.dci3"
fileName6:
        .asciz  "/images/head1.dci3"
fileName7:
        .asciz  "/images/head2.dci3"
fileName8:
        .asciz  "/images/head3.dci3"
fileName9:
        .asciz  "/images/bar.dci3"

        .align  4
file1:
        .incbin "/images/brickskull.dci3"
fileEnd1:

        .align  4
file2:
        .incbin "/images/rock.dci3"
fileEnd2:

        .align  4
file3:
        .incbin "/images/rockskull.dci3"
fileEnd3:

        .align  4
file4:
        .incbin "/images/brick.dci3"
fileEnd4:

        .align  4
file5:
        .incbin "/images/pistol.dci3"
fileEnd5:

        .align  4
file6:
        .incbin "/images/head1.dci3"
fileEnd6:

        .align  4
file7:
        .incbin "/images/head2.dci3"
fileEnd7:

        .align  4
file8:
        .incbin "/images/head3.dci3"
fileEnd8:

        .align  4
file9:
        .incbin "/images/bar.dci3"
fileEnd9:


        .align  4

        .global fileName
fileName:
        .long   fileName1
        .long   fileName2
        .long   fileName3
        .long   fileName4
        .long   fileName5
        .long   fileName6
        .long   fileName7
        .long   fileName8
        .long   fileName9

        .global fileSize
fileSize:
        .long   fileEnd1 - file1
        .long   fileEnd2 - file2
        .long   fileEnd3 - file3
        .long   fileEnd4 - file4
        .long   fileEnd5 - file5
        .long   fileEnd6 - file6
        .long   fileEnd7 - file7
        .long   fileEnd8 - file8
        .long   fileEnd9 - file9

        .global filePtr
filePtr:
        .long   file1
        .long   file2
        .long   file3
        .long   file4
        .long   file5
        .long   file6
        .long   file7
        .long   file8
        .long   file9

        .align  4
