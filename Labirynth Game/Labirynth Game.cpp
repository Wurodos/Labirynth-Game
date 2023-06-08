#include <iostream>
#include <cstdlib>
#include <string>
#include <windows.h>

const int MAP_SIZE = 10;

bool hasTreasure;
int turn = 100;

inline int opp(int dir)
{
	switch (dir)
	{
	case 0: return 2;
	case 1: return 3;
	case 2: return 0;
	default: return 1;
	}
}


class space
{
protected:
	std::string name;
	std::string desc;
	char type;
	void tell() { std::cout << "\n" << desc << "\n"; }
public:
	bool hasWall[4]; // ->, ^, <-, v
	space() 
	{
		hasWall[0] = true;
		hasWall[1] = true;
		hasWall[2] = true;
		hasWall[3] = true;
		type = '$';
	}
	void printName();
	char getType() { return type; }
	virtual void EffectOnEnter() { return; }
};

space* map[MAP_SIZE][MAP_SIZE];
int playerX = -1, playerY = -1, grenades = 0;

class empty_space : public space
{
public:
	empty_space() { name = "пустой комнате"; desc = "Здесь ничего нет"; type = '-'; }
	void EffectOnEnter() { tell(); }
};

class hole : public space
{
	int toX, toY;
public:
	hole() { toX = 0; toY = 0; name = "яме"; desc = "Переносит вас в яму со следующим номером"; type = 'H'; }
	hole(int x, int y) { toX = x; toY = y; name = "яме"; desc = "Переносит вас в яму со следующим номером"; type = 'H'; }
	void Link(int x, int y) { toX = x; toY = y; }
	void EffectOnEnter() { tell(); playerX = toX; playerY = toY; }
};

class arsenal : public space
{
	bool isFull;
public:
	arsenal() { name = "арсенале"; desc = "Восстановите ваш запас гранат (1 раз за игру)"; type = 'A'; isFull = true; }
	void EffectOnEnter() { if (isFull) { tell(); grenades = 3; isFull = false; } else std::cout << "\nАрсенал пуст!\n"; }
};

class treasure : public space
{
public:
	treasure() { name = "комнате с СОКРОВИЩЕМ!"; desc = "Вы подобрали сокровище"; type = 'T'; }
	void EffectOnEnter() { if (!hasTreasure) { tell(); hasTreasure = true; turn += 100; } else { std::cout << "\nВы его уже подобрали\n"; } }
};

class river : public space
{
private:
	int dir;
	void MoveInDir()
	{
		switch (dir)
		{
		case 0:
			playerX++;
			break;
		case 1:
			playerY++;
			break;
		case 2:
			playerX--;
			break;
		case 3:
			playerY--;
			break;
		default:
			std::cout << "Вы достигли истока или устья реки\n";
			break;
		}
	}
public:
	river(int dir, int pDir)
	{
		name = "реке"; desc = "Каждый ход будет уносить вас на 2 клетки вперед. Против течения идти нельзя!";

		switch (dir)
		{
		case 0: type = '>'; break;
		case 1: type = '^'; break;
		case 2: type = '<'; break;
		case 3: type = 'v'; break;
		default:
			type = 'R';
		}
		this->dir = dir;
		if (dir >= 0)
		{
			hasWall[dir] = false;
		}
		hasWall[opp(pDir)] = false;
	}
	void EffectOnEnter() {
		tell();
		MoveInDir();
		if (type != 'R')
		((river*)map[playerX][playerY])->MoveInDir();
	}
};

void CreateMap();
void Game();

int main()
{
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);
	srand(time(0));
	
	hasTreasure = false;
	CreateMap();
	Game();

	// 4. View (Debug)

	// No walls
	for (int i = MAP_SIZE - 1; i >= 0; i--)
	{
		for (int j = 0; j < MAP_SIZE; j++)
		{
			std::cout << map[j][i]->getType();
		}
		std::cout << "\n";
	}

	// w/ walls
	std::cout << "\n\n\n";
	for (int j = 0; j < MAP_SIZE; j++)
		if (map[j][MAP_SIZE - 1]->hasWall[1]) std::cout << " _"; else std::cout << "  ";
	std::cout << "\n";
	for (int i = MAP_SIZE - 1; i >= 0; i--)
	{
		if (map[0][i]->hasWall[2]) std::cout << "|"; else std::cout << " ";
		for (int j = 0; j < MAP_SIZE; j++)
		{
			if (map[j][i]->hasWall[3]) std::cout << "_"; else std::cout << " ";
			if (map[j][i]->hasWall[0]) std::cout << "|"; else std::cout << " ";
		}
		std::cout << "\n";
	}


	for (int i = 0; i < MAP_SIZE; i++)
		for (int j = 0; j < MAP_SIZE; j++)
			delete map[i][j];
}

void space::printName()
{
	std::cout << "Сейчас вы в " << name << "\n";
}

void CreateMap()
{


	// 1. RIVER (breaks walls)
	// 1.1 Choose a side
	int startSide = rand() % 4,
		startPos = rand() % (MAP_SIZE - 2) + 1,
		x, y;
	switch (startSide)
	{
	case 0: x = MAP_SIZE - 1; y = startPos; break;
	case 1: x = startPos; y = MAP_SIZE-1; break;
	case 2: x = 0; y = startPos; break;
	case 3: x = startPos; y = 0; break;
	}

    int pDir = opp(startSide);

	map[x][y] = new river(-1, startSide);

	while (1)
	{
		//std::cout << x << " " << y << "\n";
		switch (pDir)
		{
		case 0: x++; break;
		case 1: y++; break;
		case 2: x--; break;
		case 3: y--; break;
		}
		if (((x > 0) && (x < MAP_SIZE-1)) && ((y > 0) && (y < MAP_SIZE-1)))
		{
			int dir = rand() % 4;
			if ((dir == startSide) || (dir == opp(pDir)))
			{
				dir = opp(startSide);
			}
			map[x][y] = new river(dir, pDir);

			pDir = dir;
		}
		else
		{
			map[x][y] = new river(-2, pDir);
			break;
		}
	}
    
	// 2. ARSENAL, TREASURE, HOLES
		while (1)
		{
			x = rand() % MAP_SIZE;
			y = rand() % MAP_SIZE;
			if (!map[x][y])
			{
				map[x][y] = new arsenal;
				break;
			}
		}

		while (1)
		{
			x = rand() % MAP_SIZE;
			y = rand() % MAP_SIZE;
			if (!map[x][y])
			{
				map[x][y] = new treasure;
				break;
			}
		}



		int prevX, prevY, X, Y;
		while (1)
		{
			x = rand() % MAP_SIZE;
			y = rand() % MAP_SIZE;
			if (!map[x][y])
			{
				map[x][y] = new hole;
				break;
			}
		}
		X = prevX = x; Y = prevY = y;
		for (int i = 0; i < 7; i++)
			while (1)
			{
				x = rand() % MAP_SIZE;
				y = rand() % MAP_SIZE;
				if (!map[x][y])
				{
					map[x][y] = new hole(prevX, prevY);
					prevX = x;
					prevY = y;
					break;
				}
			}
		((hole*)map[X][Y])->Link(prevX, prevY);




	for (int i = 0; i < MAP_SIZE; i++)
		for (int j = 0; j < MAP_SIZE; j++)
			if (!map[i][j]) map[i][j] = new empty_space;

	// 3. Break the vertical and then horizontal walls

	for (int i=MAP_SIZE - 1; i > 0; i--)
		for (int j = 0; j < MAP_SIZE; j++)
		{
			if (rand() % 2)
			{
				map[j][i]->hasWall[3] = false;
				map[j][i - 1]->hasWall[1] = false;
			}
		}

	for (int i = MAP_SIZE - 1; i >= 0; i--)
		for (int j = 0; j < MAP_SIZE - 1; j++)
		{
			if (rand() % 2)
			{
				map[j][i]->hasWall[0] = false;
				map[j + 1][i]->hasWall[2] = false;
			}
		}

	// 3.2 Exit. Just break random wall somewhere on the edge

	startSide = rand() % 4;
	startPos = rand() % MAP_SIZE;
	switch (startSide)
	{
	case 0: map[MAP_SIZE - 1][startPos]->hasWall[0] = false; break;
	case 1: map[startPos][MAP_SIZE-1]->hasWall[1] = false; break;
	case 2: map[0][startPos]->hasWall[2] = false; break;
	case 3: map[startPos][0]->hasWall[3] = false; break;
	}

}

void Game()
{
	playerX = rand() % MAP_SIZE;
	playerY = rand() % MAP_SIZE;
	bool loop = true;
	int ch;
	grenades = 3;

	while (loop)
	{
		//std::cout << playerX << " " << playerY << "\n";
		if (((playerX < 0) || (playerX >= MAP_SIZE)) || ((playerY < 0) || (playerY >= MAP_SIZE)))
		{
			std::cout << "Вы победили!!!";
			return;
		}
		if (turn == 0)
		{
			std::cout << "Вы проиграли...";
			return;
		}
		turn--;
		std::cout << "--------------------------------\n";
		map[playerX][playerY]->printName();
		map[playerX][playerY]->EffectOnEnter();
		std::cout << "У вас осталось " << grenades << " гранат и " << turn << " ходов\n";
		std::cout << "--------------------------------\n\n\n\n\n";

		std::cout << "\nВыберите:\n1) Вправо\n2) Вверх\n3) Влево\n4) Вниз\n5) Бросить гранату\n6) Остаться на месте\n0) Завершить игру досрочно\n";
		std::cin >> ch;
		switch (ch)
		{
		case 1:
			if (map[playerX][playerY]->hasWall[0])
				std::cout << "\n\n\n\t\tПрохода нет.\n\n\n";
			else playerX++;
			break;
		case 2:
			if (map[playerX][playerY]->hasWall[1])
				std::cout << "\n\n\n\t\tПрохода нет.\n\n\n";
			else playerY++;
			break;
		case 3:
			if (map[playerX][playerY]->hasWall[2])
				std::cout << "\n\n\n\t\tПрохода нет.\n\n\n";
			else playerX--;
			break;
		case 4:
			if (map[playerX][playerY]->hasWall[3])
				std::cout << "\n\n\n\t\tПрохода нет.\n\n\n";
			else playerY--;
			break;
		case 5:
			if (grenades < 1)
			{
				std::cout << "У вас закончились гранаты!";
				break;
			}
			std::cout << "\n1) Вправо\n2) Вверх\n3) Влево\n4) Вниз\n";
			std::cin >> ch;
			ch--;
			if (!map[playerX][playerY]->hasWall[ch])
				std::cout << "В этой стороне не было стены\n";
			else
			{
				switch (ch)
				{
				case 0:
					if (playerX + 1 != MAP_SIZE)
					{
						map[playerX][playerY]->hasWall[0] = false;
						map[playerX + 1][playerY]->hasWall[2] = false;
						grenades--;
						break;
					}
				case 1:
					if (playerY + 1 != MAP_SIZE)
					{
						map[playerX][playerY]->hasWall[1] = false;
						map[playerX][playerY + 1]->hasWall[3] = false;
						grenades--;
						break;
					}
				case 2:
					if (playerX != 0)
					{
						map[playerX][playerY]->hasWall[2] = false;
						map[playerX - 1][playerY]->hasWall[0] = false;
						grenades--;
						break;
					}
				case 3:
					if (playerY != 0)
					{
						map[playerX][playerY]->hasWall[3] = false;
						map[playerX][playerY - 1]->hasWall[1] = false;
						grenades--;
						break;
					}
					std::cout << "Это неразрушимая стена\n";

				}
			}
			break;
		case 6:
			break;
		default:
			return;
		}
	}
}