#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>
#include"socketKhai.h"
#include <windows.h>

using namespace std;

//use MACRO -> Nice
#define FOR_ALL_POINT_I_J for (int i = 0; i < boardSize; i++)\
	for (int j = 0; j < boardSize; j++) 

const string mapPlayer1 = "mapPlayer1.txt";
const string mapPlayer2 = "mapPlayer2.txt";

//some symbols for the game
const char isWATER = '0';
const char isHIT = 'X';
const char isMISS = 'M';

//some colors for the game
const char blue[] = "\x1B[34m";
const char normal[] = "\x1B[0m";
const char bblue[] = "\x1B[94m";
const char red[] = "\x1B[31m";
const char yellow[] = "\x1B[93m";

class CORD {
public:
	int x;
	int y;
	CORD(int x, int y) {
		this->x = x;
		this->y = y;
	}
};

struct PLAYER {
	string map;
	char** grid = NULL;
	int boom = 0;
	int score = 0;
};

int boardSize = 0;
int isServer, notServer;
SOCKET server, client;

int SelectMode() {
	cout << endl << "\n_________" << bblue << "****" << red << " B A T T L E S H I P S " << bblue << "****" << normal << "_________" << endl;
	cout << endl << "\n\n	     GAME MODE:" << endl;
	cout << "-------------------------------------";
	cout << "\n|  1. HUMAN VS HUMAN OFFLINE MATCH  |" << endl;
	cout << "|  2. HUMAN VS AI MATCH             |" << endl;
	cout << "|  3. HUMAN VS HUMAN ONLINE MATCH   |" << endl;
	cout << "-------------------------------------" << endl;
	int ch = 0;

	while (ch != 1 && ch != 2 && ch != 3) {
		cout << endl << "To start, please select one game mode...	" << endl;
		cin >> ch;
	}

	return ch;
}

bool LoadMap(PLAYER* player) {

	ifstream fileInput(player->map);
	if (fileInput.fail()) return false;
	fileInput >> boardSize; //Load the size of map(board)

	if (boardSize <= 0) return false;

	//Creat dynamic memory
	player->grid = new char* [boardSize];
	for (int i = 0; i < boardSize; i++) {
		(player->grid)[i] = new char[boardSize];
	}
	//Read each point in map of player
	while (!fileInput.eof()) {
		FOR_ALL_POINT_I_J{
				fileInput >> player->grid[i][j];
		}
	}

	fileInput.close();
	return true;
}

bool IsNewShip(vector<char> shipName, char thisPos) {

	for (int k = 0; k < shipName.size(); k++)
		if ((thisPos == shipName.at(k))) return false;

	if (thisPos != isWATER) return true;
	return false;
}

void DrawBoard(int thisPlayer, PLAYER player, vector<char> shipName)
{
	//Draws the board for a player (thisPlayer)
	cout << "\nPLAYER " << thisPlayer << "'s GAME BOARD\n";
	cout << "----------------------\n";

	//Loop through top row (board_width) and number columns
	cout << "   ";
	for (int w = 0; w < boardSize; ++w) {
		if (w < 10)
			//Numbers only 1 character long, add two spaces after
			cout << w << "  ";
		else if (w >= 10)
			//Numbers 2 characters long, add only 1 space after
			cout << w << " ";
	}
	cout << "\n";

	//Loop through each grid point and display to console
	for (int h = 0; h < boardSize; ++h) {
		for (int w = 0; w < boardSize; ++w) {

			//If this is the FIRST (left) grid point, number the grid first
			if (w == 0) cout << h << " ";
			//If h was 1 character long, add an extra space to keep numbers lined up
			if (w < 10 && w == 0) cout << " ";

			//Don't show ships, BUT show damage if it's hit
			int logic = 1;

			for (int i = 0; i < shipName.size(); i++) logic = logic && (player.grid[h][w] != shipName.at(i));
			if (logic)
			{
				if (player.grid[h][w] == isHIT) cout << red << player.grid[h][w] << normal << "  ";
				else if (player.grid[h][w] == isMISS) cout << yellow << player.grid[h][w] << normal << "  ";
				else cout << bblue << player.grid[h][w] << normal << "  ";
			}

			else
			{
				cout << bblue << isWATER << normal << "  ";
			}
			//If we have reached the border.. line feed
			if (w == boardSize - 1) cout << "\n";
		}
	}
	cout << "----------------------\n";
}

bool UserInputAttack(int& x, int& y, int theplayer, int gameMode, PLAYER enemyPlayer)
{
	bool goodInput = false;
	if (gameMode == 2 && theplayer == 2) {
		cout << "\nAI IS ATTACKING ..";
		//Your mission is to creat x,y for AI
		Sleep(1000);
		cout << "..";
		Sleep(1000);
		cout << "..";
		Sleep(1000);
		{

			srand(time(NULL));
			vector<CORD> cord; //save all empty point to a vector to select from

			//first save all black points
			FOR_ALL_POINT_I_J
				if (enemyPlayer.grid[i][j] != isHIT && enemyPlayer.grid[i][j] != isMISS)
					if (((i % 2 == 1) && (j % 2 == 1)) || ((i % 2 == 0) && (j % 2 == 0)))
						cord.push_back(CORD(i, j));

			//if there are no black points, choose normally
			if (cord.size() == 0) {
				FOR_ALL_POINT_I_J
					if (enemyPlayer.grid[i][j] != isHIT && enemyPlayer.grid[i][j] != isMISS)
						cord.push_back(CORD(i, j));
			}

			int hunt = rand() % cord.size(); //random a point in vector
			x = cord.at(hunt).x;
			y = cord.at(hunt).y;
		}

		goodInput = true;
		return goodInput;
	}
	cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO ATTACK: ";
	cin >> x >> y; //the first one is horizontal
	if (x < 0 || x >= boardSize) return goodInput;
	if (y < 0 || y >= boardSize) return goodInput;
	goodInput = true;
	return goodInput;
}

bool UserInputAttackOnline(int& x, int& y, int thePlayer, SOCKET server, SOCKET client)
{
	bool goodInput = false;
	SOCKET target; 
	int turn;

	if (isServer == 1) { target = client; turn = 1; }

	else { target = server; turn = 2; }
	
	//if it is your turn
	if (thePlayer == turn) {
		cout << "\nPLAYER " << thePlayer << ", ENTER COORDINATES TO ATTACK: ";

		cin >> x >> y; //the first one is horizontal
		if (x < 0 || x >= boardSize) return goodInput;
		if (y < 0 || y >= boardSize) return goodInput;
		SendCord(target, x, y);
		goodInput = true;

	}
	//if it is your enemy turn
	else {
		cout << "\nPLAYER " << thePlayer << " IS ENTERING COORDINATES TO ATTACK...\n-----PLEASE WAIT-----";
		Sleep(1000);
		ReceiveCord(target, x, y);
		goodInput = true;
	}
	return goodInput;
}

int GameOverCheck(PLAYER thisPlayer, int thisIndex, PLAYER enemyPlayer, int enemyIndex, vector<char> shipName)
{
	int winner = thisIndex;
	//Loop through enemy board
	for (int w = 0; w < boardSize; ++w) {
		for (int h = 0; h < boardSize; ++h) {
			//If any ships remain, game is NOT over
			for (int k = 0; k < shipName.size(); k++) if (shipName.at(k) == enemyPlayer.grid[h][w])
			{
				winner = 0;
			}
		}
	}
	if ((enemyPlayer.boom == 0) && (thisPlayer.boom == 0))
		if (thisPlayer.score > enemyPlayer.score) winner = thisIndex;
		else if (thisPlayer.score < enemyPlayer.score) winner = enemyIndex;
		else {
			winner = 3;
		}

	//If we get here, thisPlayer won, game over!
	return winner;
}

void Result(int aWin, PLAYER* player, int gameMode) {
	system("cls");

	cout << yellow;

	if (aWin == 3) {
		cout << "\n\n	WE HAVE A TIE GAME!!! \n\n\n\n";
	}

	if (gameMode == 1) {
		PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
		cout << "\n\n	CONGRATULATIONS!!!  PLAYER " << aWin << " HAS WON THE GAME!\n\n\n\n";
	}
	if (gameMode == 2) {
		if (2 == aWin) {
			PlaySound(TEXT("lose.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\n	YOU HAVE LOST TO AI!!! \n\n\n\n";
		}
		if (1 == aWin) {
			PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\n	CONGRATULATIONS!!! YOU HAVE WON AI!!! \n\n\n\n";
		}
	}
	
	if (gameMode == 3) {
		if (aWin == notServer) {
			PlaySound(TEXT("lose.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\n	YOU LOSE! ENEMY PLAYER HAS WON THE GAME!\n\n\n\n";
		}
		if (aWin == isServer) {
			PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\n	CONGRATULATIONS!!!  YOU HAS WON THE GAME!\n\n\n\n";
		}
	}
	cout << normal;

	cout << "\nHope you enjoy this game. Thanks for your time.\n\n";
	
	system("pause");
}

int SizeOfShip(PLAYER player1, char shipName) {
	int count = 0;
	FOR_ALL_POINT_I_J if (player1.grid[i][j] == shipName) count++;
	return count;
}

bool IsPlaceable(int x, int y, int dir, PLAYER& player2, char shipName, int shipSize) {
	bool logic = true;
	srand(time(NULL));

	if (dir == 0)
	{
		int left = rand() % 2;
		if (left == 1) {
			for (int i = y - shipSize + 1; i < y + 1; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[x][i] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = y - shipSize + 1; i < y + 1; i++) player2.grid[x][i] = shipName;
		}
		else {
			for (int i = y; i < y + shipSize; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[x][i] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = y; i < y + shipSize; i++) player2.grid[x][i] = shipName;
		}
	}

	if (dir == 1)
	{
		int up = rand() % 2;
		if (up == 0) {
			for (int i = x - shipSize + 1; i < x + 1; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[i][y] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = x - shipSize + 1; i < x + 1; i++) player2.grid[i][y] = shipName;
		}
		else {
			for (int i = x; i < x + shipSize; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[i][y] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = x; i < x + shipSize; i++) player2.grid[i][y] = shipName;
		}
	}
	return logic;
}

void CreatMapAI(PLAYER player1, PLAYER& player2, vector<char> shipName) {

	//Your mission is to creat a mapPlayer2.txt which has AI's map, depending on mapPlayer1.txt

	vector<int> shipSize; //take all the sizes of ships to a equivalent vector like shipName
	for (int thisShip = 0; thisShip < shipName.size(); thisShip++) shipSize.push_back(SizeOfShip(player1, shipName.at(thisShip)));

	srand(time(NULL));
	int x, y, dir;
	for (int thisShip = 0; thisShip < shipName.size(); thisShip++) {
		do {
			x = rand() % boardSize; //the first set point of a ship
			y = rand() % boardSize;
			dir = rand() % 2; //0 is horizontal, 1 is vertical

		} while (!IsPlaceable(x, y, dir, player2, shipName.at(thisShip), shipSize.at(thisShip)));
	}
}

bool ServerConfig(SOCKET& client, SOCKET& server) {
	WSADATA WSAData;
	SOCKADDR_IN serverAddr, clientAddr;
	bool run = false;

	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server = socket(AF_INET, SOCK_STREAM, 0);

	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5555);

	bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	listen(server, 0);

	system("cls");
	system("C:\\Windows\\System32\\ipconfig"); //print out ipconfig console, to see all IP
	cout << red<<"\nTELL YOUR RIVAL YOUR IPV4 ADDRESS, WHICH APPEARS IN THE FIRST SCREEN" << normal<< endl;

	cout << "\nListening for incoming connections..." << endl;

	int clientAddrSize = sizeof(clientAddr);
	if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
	{
		cout << "\nClient connected!" << endl;
		run = true;
	}
	else {
		cout << "\nCan't connect to client" << endl;
		run = false;
	}
	cout << "\nPLEASE CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
	system("pause");
	ReceiveFile(client, "mapPlayer2.txt");
	Sleep(500);
	SendFile(client, "mapPlayer1.txt");
	Sleep(500);
	return run;
}

bool ClientCofig(SOCKET& client, SOCKET& server) {
	WSADATA WSAData;
	SOCKADDR_IN addr;
	string serverIP;

	WSAStartup(MAKEWORD(2, 0), &WSAData);
	server = socket(AF_INET, SOCK_STREAM, 0);
	system("cls");
	serverIP.assign(GetIP()); //get IP of server, server must TELL client

	addr.sin_addr.s_addr = inet_addr(serverIP.c_str()); // replace the ip with your futur server ip address. If server AND client are running on the same computer, you can use the local ip 127.0.0.1
	addr.sin_family = AF_INET;
	addr.sin_port = htons(5555);

	connect(server, (SOCKADDR*)&addr, sizeof(addr));
	cout << "\nConnected to server!" << endl;

	cout << "\nPLEASE CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
	system("pause");
	SendFile(server, "mapPlayer2.txt");
	Sleep(500);
	ReceiveFile(server, "mapPlayer1.txt");
	Sleep(500);
	return true;
}

bool Attacking(int& thisPlayer, int& aWin, vector<char>& shipName, PLAYER* player, int gameMode) {
	//Because we are ATTACKING now, the opposite players board is the display board
	int enemyPlayer;
	if (thisPlayer == 1) enemyPlayer = 2;
	if (thisPlayer == 2) enemyPlayer = 1;
	system("cls");
	DrawBoard(enemyPlayer, player[enemyPlayer], shipName);

	cout << endl << "PLAYER " << thisPlayer << "'S SCORE: " << player[thisPlayer].score << endl;
	cout << "PLAYER " << thisPlayer << "'S BOOMS LEFT: " << player[thisPlayer].boom << endl;
	//Get attack coords from this player
	bool goodInput = false;
	int x = 0, y = 0;
	while (goodInput == false) {
		if (gameMode == 3)	goodInput = UserInputAttackOnline(x, y, thisPlayer, server, client);
		else goodInput = UserInputAttack(x, y, thisPlayer, gameMode, player[enemyPlayer]);
	}
	player[thisPlayer].boom--;
	//Check board; if a ship is there, set as HIT.. otherwise MISS
	for (int thisShip = 0; thisShip < shipName.size(); thisShip++)
		if (player[enemyPlayer].grid[x][y] == shipName.at(thisShip))
		{
			//nice shot
			PlaySound(TEXT("Explosion+2.wav"), NULL, SND_FILENAME | SND_ASYNC);
			//Sleep(3500);
			FOR_ALL_POINT_I_J
				if (player[enemyPlayer].grid[i][j] == shipName.at(thisShip)) {
					player[enemyPlayer].grid[i][j] = isHIT;
					player[thisPlayer].score++;
				}
			break;
		}
	if (player[enemyPlayer].grid[x][y] == isWATER) player[enemyPlayer].grid[x][y] = isMISS;

	system("cls");
	DrawBoard(enemyPlayer, player[enemyPlayer], shipName);
	cout << endl;
	system("pause");

	//Check to see if the game is over
	//If 0 is returned, nobody has won yet
	aWin = GameOverCheck(player[thisPlayer], thisPlayer, player[enemyPlayer], enemyPlayer, shipName);
	if (aWin != 0) return false;

	//Alternate between each player as we loop back around
	thisPlayer = (thisPlayer == 1) ? 2 : 1;
	return true;
}


int main()
{
	PLAYER* player = new PLAYER[3]; //Use player 1 & 2, 0 is ignored
	player[1].map = mapPlayer1;
	player[2].map = mapPlayer2;
	int thisPlayer = 1;
	int aWin = 0;
	int gameMode = SelectMode();
	if (gameMode == 3) {

		cout << "\n<If you are player 1, you will be server. Otherwise, if you are player 2, you will be client !!>" << endl
			<< "\nWhich player do you want to be (1/2)?"<<endl;
		cin >> isServer;
		notServer = (isServer == 1) ? 2 : 1;

		if (isServer == 1) ServerConfig(client, server); else ClientCofig(client, server);

		if (!LoadMap(&player[isServer])) {
			cout << "\nFailed to load player " << isServer << "'s map!" << endl;
			return -1;
		}
		vector<char> shipName; //vector saves types of ship
		shipName.push_back('1');

		FOR_ALL_POINT_I_J{
					if (IsNewShip(shipName, player[isServer].grid[i][j])) shipName.push_back(player[isServer].grid[i][j]);
		}
		player[isServer].boom = (player[notServer].boom = boardSize * boardSize / 2); //config the number of turns = boom

		if (!LoadMap(&player[notServer])) {
			cout << "\nFailed to load player " << notServer << "'s map!" << endl;
			return -1;
		}

		cout << "\nPLAYERS' MAPS LOADED, READY TO FIGHT!!!" << endl;
		system("pause");


		//Ready to play the game


		do {} while (Attacking(thisPlayer, aWin, shipName, player, gameMode));

	}
	else {

		cout << "\nPLEASE SECRETLY CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
		system("pause");
		if (!LoadMap(&player[1])) {
			cout << "\nFailed to load player 1's map!" << endl;
			return -1;
		}
		vector<char> shipName; //vector saves types of ship
		shipName.push_back('1');

		FOR_ALL_POINT_I_J{
					if (IsNewShip(shipName, player[1].grid[i][j])) shipName.push_back(player[1].grid[i][j]);
		}
		player[1].boom = (player[2].boom = boardSize * boardSize / 2); //config the number of turns = boom

		if (gameMode == 2) {
			player[2].grid = new char* [boardSize];
			for (int i = 0; i < boardSize; i++) {
				(player[2].grid)[i] = new char[boardSize];
			}
			FOR_ALL_POINT_I_J player[2].grid[i][j] = isWATER;

			cout << "\nAI IS CREATING ITS FLEE ON THE MAP. PLEASE WAIT A MINUTE...\n";

			CreatMapAI(player[1], player[2], shipName);
		}
		else if (!LoadMap(&player[2])) {
			cout << "\nFailed to load player 2's map!" << endl;
			return -1;
		}

		cout << "\nPLAYERS' MAPS LOADED, READY TO FIGHT!!!" << endl;
		system("pause");

		//Ready to play the game
		do {} while (Attacking(thisPlayer, aWin, shipName, player, gameMode));


	}
	Result(aWin, player, gameMode);
	delete[](player[1].grid);
	delete[](player[2].grid);
	return 0;
}