#!/bin/bash

# init the variables of the tennis game
# the board is a 2D array
game_on=true
playerOneScore=50
start=1
playerTwoScore=50
playerTwoInput=""
playerOneInput=""
middleRow=' |       |       O       |       | '

# the ball position is a variable : -1,-2,3, 1, 2, 3 ,at the beginning the ball is at the center of the board
ballPosition=0
#print the board
function printBoard() {
    # if this is the first time to print the board , print the instructions
    echo " Player 1: $playerOneScore         Player 2: $playerTwoScore "
    echo " --------------------------------- "
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo "$middleRow"
    echo " |       |       #       |       | "
    echo " |       |       #       |       | "
    echo " --------------------------------- "
    # if this is  not the first time to print the board , print the instructions:

if [[ $start -eq 0 ]]; then
    echo -e "       Player 1 played: ${playerOneInput}\n       Player 2 played: ${playerTwoInput}\n\n"
fi
start=0

}


# update the board
function updateBoard(){
    # update the ball position using switch case , the ball position is a variable : -1,-2,3, 1, 2, 3 ,at the beginning the ball is at the center of the board
    case $ballPosition in
        -3)
            middleRow='O|       |       #       |       | '
            ;;
        -2)
            middleRow=' |   O   |       #       |       | '
            ;;
        -1)
            middleRow=' |       |   O   #       |       | '
            ;;
        0)
            middleRow=' |       |       O       |       | '
            ;;
        1)
            middleRow=' |       |       #   O   |       | '
            ;;
        2)
            middleRow=' |       |       #       |   O   | '
            ;;
        3)
            middleRow=' |       |       #       |       |O'
            ;;
        *) 
            ;;
    esac
}


# playerPick
# 1 : player 1
# 2 : player 2
function playerPick(){
    #read the input from the player1
    read -p "PLAYER 1 PICK A NUMBER: " -s playerOneInput
    echo
    # calculate the score of the player

    # check if the input is a number
    while ! [[ $playerOneInput =~ ^[0-9]+$ ]] || ! [[ $((playerOneScore - playerOneInput)) -ge 0 ]]
    do
        echo "NOT A VALID MOVE !"
        read -p "PLAYER 1 PICK A NUMBER: " -s playerOneInput  
        echo
    done
    # update the score of the player
    playerOneScore=$((playerOneScore - playerOneInput))
    #read the input from the player2
    read -p "PLAYER 2 PICK A NUMBER: " -s playerTwoInput
    echo
    # check if the input is a number
    while ! [[ $playerTwoInput =~ ^[0-9]+$ ]] || ! [[ $((playerTwoScore - playerTwoInput)) -ge 0 ]]
    do
        echo "NOT A VALID MOVE !"
        read -p "PLAYER 2 PICK A NUMBER: " -s playerTwoInput  
        echo
    done
    # update the score of the player
    playerTwoScore=$((playerTwoScore - playerTwoInput))

    # if [[ $playerOneInput -eq $playerTwoInput ]]; then
    #     continue
    

    # if player one lost the round
    if [[ $playerOneInput -lt $playerTwoInput ]]; then
    # if player one is losing
        if [[ $ballPosition -le 0 ]]; then
            ballPosition=$((ballPosition - 1))
        else
            ballPosition=-1
        fi

    # if player two lost the round
    elif [[ $playerTwoInput -lt $playerOneInput ]]; then
        # if player two is losing
        if [[ $ballPosition -ge 0 ]]; then
            ballPosition=$((ballPosition + 1))
        # if player one is losing
        else
            ballPosition=1
        fi
    fi

    updateBoard
}
# check if there is a winner
checkWinner(){
    if [[ $ballPosition = -3 ]] || [[ $ballPosition = 3 ]]; then
        game_on=false
        printBoard
        if [[ $ballPosition = -3 ]]; then
            echo "PLAYER 2 WINS !"
        else
            echo "PLAYER 1 WINS !"
        fi  
    else 
        if [[ $playerOneScore -eq 0 ]] && [[ $playerTwoScore -gt 0 ]]; 
            then
                printBoard
                echo "PLAYER 2 WINS !"
                game_on=false
        elif [[ $playerOneScore -gt 0 ]] && [[ $playerTwoScore = 0 ]]; then
                printBoard
                echo "PLAYER 1 WINS !"
                game_on=false
            elif [[ $playerOneScore = 0 ]] && [[ $playerTwoScore = 0 ]]; then
                    printBoard
                if [[ $ballPosition -gt 0 ]]; then
                    echo "PLAYER 1 WINS !"
                    game_on=false
                elif [[ $ballPosition -lt 0 ]]; then
                    echo "PLAYER 2 WINS !"
                    game_on=false
                else 
                    echo "IT'S A DRAW !"
                    game_on=false
                fi
            fi
    fi
        
}



# call the functions while the game is on
while $game_on
do
    printBoard
    playerPick
    checkWinner
done
