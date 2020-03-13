#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <new>
#include <cstdlib>
#include <ctime>

#define SCREEN_WIDTH 640 // 窗口宽
#define SCREEN_HEIGHT 480 // 窗口高

#define BODYNUMS 1000 // 总人数
#define INFECTNUMS 10 // 初始感染人数
#define HOSPITAL 100 // 医院总床位

#define INCUP 60 // 潜伏期时间，单位：帧，一般一秒六十帧
#define HOSPITALRESPTIME 30 // 医院反应时间，单位同上
#define HEALTIME 90 // 入院后治疗时间，单位同上
#define DIETIME 180 // 未及时入院死亡时间，单位同上

#define QUADTREE_MAX_OBJECTS 10 // 四叉树最大物体数量
#define QUADTREE_MAX_LEVELS 5 // 四叉树最大层数

#ifndef FONTPATH
#define FONTPATH "C:/Windows/Fonts/msyh.ttc"
#endif

// ---这条线以下不用再修改了---

#define BODYSIZE 2

#define STR(N) #N
#define CAT(S, N) S STR(N)

const unsigned int BORDERX = SCREEN_WIDTH - BODYSIZE;
const unsigned int BORDERY = SCREEN_HEIGHT - BODYSIZE;

const unsigned int RX = BORDERX - 1;
const unsigned int RY = BORDERY - 1;

const SDL_Color red = {255, 0, 0};
const SDL_Color yellow = {255, 255, 0};
const SDL_Color green = {0, 255, 0};
const SDL_Color white = {255, 255, 255};
const SDL_Color black = {0, 0, 0};

struct Point
{
	int x, y;
	int dx, dy;
	unsigned int wait, die;
	unsigned char state;
};

class Quadtree
{
	public:
		Quadtree() {}
		Quadtree(unsigned int plevel, SDL_Rect pbounds)
		{
			level = plevel;
			bounds = pbounds;
			nodes = NULL;
		}
		Quadtree(const Quadtree &obj)
		{
			level = obj.level;
			bounds = obj.bounds;
			nodes = obj.nodes;
		}
		void clear()
		{
			objects.clear();
			if (nodes)
			{
				nodes[0].clear();
				nodes[1].clear();
				nodes[2].clear();
				nodes[3].clear();
				delete[] nodes;
				nodes = NULL;
			}
		}
		void insert(Point *p)
		{
			int index;
			if (nodes)
			{
				index = getindex(p);
				if (index != -1)
				{
					nodes[index].insert(p);
					return;
				}
			}
			objects.push_back(p);
			if (objects.size() > QUADTREE_MAX_OBJECTS && level < QUADTREE_MAX_LEVELS)
			{
				if (!nodes) split();
				std::vector<Point *>::iterator it = objects.begin();
				while (it != objects.end())
				{
					index = getindex(*it);
					if (index != -1)
					{
						nodes[index].insert(*it);
						objects.erase(it);
					}
					else
					{
						++it;
					}
				}
			}
		}
		std::vector<Point *> retrieve(Point *p)
		{
			std::vector<Point *> result;
			int index = getindex(p);
			if (index != -1 && nodes)
			{
				result = nodes[index].retrieve(p);
			}
			result.insert(result.end(), objects.begin(), objects.end());
			return result;
		}
	private:
		unsigned int level;
		std::vector<Point *> objects;
		SDL_Rect bounds;
		Quadtree *nodes;
		void split()
		{
			SDL_Rect sub = {bounds.x + bounds.w / 2, bounds.y, bounds.w / 2, bounds.h / 2};
			nodes = new Quadtree[4];
			nodes[0] = Quadtree(level + 1, sub);
			sub.x = bounds.x;
			nodes[1] = Quadtree(level + 1, sub);
			sub.y = bounds.y + sub.h;
			nodes[2] = Quadtree(level + 1, sub);
			sub.x = bounds.x + sub.w;
			nodes[3] = Quadtree(level + 1, sub);
		}
		int getindex(Point *prect)
		{
			int vmp = bounds.x + (bounds.w / 2), hmp = bounds.y + (bounds.h / 2);
			bool ontop = prect->y <= hmp && prect->y + BODYSIZE <= hmp, onbottom = prect->y >= hmp, onleft = prect->x <= vmp && prect->x + BODYSIZE <= vmp, onright = prect->x >= vmp;
			if (onleft)
			{
				if (ontop) return 1;
				else if (onbottom) return 2;
			}
			else if (onright)
			{
				if (ontop) return 0;
				else if (onbottom) return 3;
			}
			return -1;
		}
};

char *itoa(int num, char *str, int radix)
{
	char index[] = "0123456789ABCDEF";
	unsigned unum;
	int i = 0, j, k;
	if (radix == 10 && num < 0)
	{
		unum = (unsigned) - num;
		str[i++] = '-';
	}
	else unum = (unsigned)num;
	do
	{
		str[i++] = index[unum%(unsigned)radix];
		unum /= radix;
	} while(unum);
	str[i] = '\0';
	if (str[0] == '-') k = 1;
	else k = 0;
	for (j = k; j <= (i - 1) / 2; j++)
	{
		char temp;
		temp = str[j];
		str[j] = str[i-1+k-j];
		str[i-1+k-j] = temp;
	}
	return str;
}

SDL_Window *wnd = NULL;
SDL_Renderer *ren = NULL;
TTF_Font *font = NULL;
SDL_Surface *stnums, *stextc, *stextcc, *stexti, *stextth, *stextoh, *stextcu, *stextd, *snumberc, *snumbercc, *snumberi, *snumberoh, *snumbercu, *snumberd;
SDL_Texture *ttnums, *ttextc, *ttextcc, *ttexti, *ttextth, *ttextoh, *ttextcu, *ttextd, *tnumberc, *tnumbercc, *tnumberi, *tnumberoh, *tnumbercu, *tnumberd;
SDL_Rect rtnums = {0, 0}, rtextc = {0, 15}, rtextcc = {0, 30}, rtexti = {0, 45}, rtextth = {0, 60}, rtextoh = {0, 75}, rtextcu = {0, 90}, rtextd = {0, 105}, rnumberc = {60, 15}, rnumbercc = {60, 30}, rnumberi = {50, 45}, rnumberoh = {60, 75}, rnumbercu = {60, 90}, rnumberd = {40, 105}, rroot = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, renrct = {0, 0, BODYSIZE, BODYSIZE};
char numberc[6], numbercc[6], numberi[6], numberoh[6], numbercu[6], numberd[6];
Point crowd[BODYNUMS];
bool run = true;

int main(int argc, char *argv[])
{
	SDL_Event e;
	unsigned int i, infects = INFECTNUMS, tinfects = INFECTNUMS, incuping = 0, cure = 0, death = 0, ohospital = 0;
	std::vector<Point *>::iterator j;
	Quadtree root(0, rroot);
	std::vector<Point *> colld;
	srand((unsigned int)time(NULL));
	for (i = 0; i < BODYNUMS; i++)
	{
		crowd[i].x = rand() % RX + 1;
		crowd[i].y = rand() % RY + 1;
		crowd[i].dx = rand() % 2 ? 1 : -1;
		crowd[i].dy = rand() % 2 ? 1 : -1;
		crowd[i].state = 0;
	}
	for (i = 0; i < INFECTNUMS; i++)
	{
		crowd[i].state = 1;
		crowd[i].wait = HOSPITALRESPTIME;
		crowd[i].die = DIETIME;
	}
	if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		std::cerr << "Error: " << SDL_GetError();
		return -1;
	}
	if (TTF_Init() == -1)
	{
		std::cerr << "Error: " << TTF_GetError();
		SDL_Quit();
		return -1;
	}
	wnd = SDL_CreateWindow("Virus", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (!wnd)
	{
		std::cerr << "Error: " << SDL_GetError();
		TTF_Quit();
		SDL_Quit();
		return -1;
	}
	ren = SDL_CreateRenderer(wnd, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!ren)
	{
		std::cerr << "Error: " << SDL_GetError();
		SDL_DestroyWindow(wnd);
		TTF_Quit();
		SDL_Quit();
		return -1;
	}
	font = TTF_OpenFont(FONTPATH, 12);
	if (!font)
	{
		std::cerr << "Error: " << TTF_GetError();
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(wnd);
		TTF_Quit();
		SDL_Quit();
		return -1;
	}
	stnums = TTF_RenderUTF8_Solid(font, CAT("总人数：", BODYNUMS), white);
	stextc = TTF_RenderUTF8_Solid(font, "累计发病：", red);
	stextcc = TTF_RenderUTF8_Solid(font, "当前发病：", red);
	stexti = TTF_RenderUTF8_Solid(font, "潜伏期：", yellow);
	stextth = TTF_RenderUTF8_Solid(font, CAT("医院总床位：", HOSPITAL), white);
	stextoh = TTF_RenderUTF8_Solid(font, "占用床位：", white);
	stextcu = TTF_RenderUTF8_Solid(font, "累计治愈：", green);
	stextd = TTF_RenderUTF8_Solid(font, "死亡：", black);
	ttnums = SDL_CreateTextureFromSurface(ren, stnums);
	ttextc = SDL_CreateTextureFromSurface(ren, stextc);
	ttextcc = SDL_CreateTextureFromSurface(ren, stextcc);
	ttexti = SDL_CreateTextureFromSurface(ren, stexti);
	ttextth = SDL_CreateTextureFromSurface(ren, stextth);
	ttextoh = SDL_CreateTextureFromSurface(ren, stextoh);
	ttextcu = SDL_CreateTextureFromSurface(ren, stextcu);
	ttextd = SDL_CreateTextureFromSurface(ren, stextd);
	rtnums.w = stnums->w;
	rtnums.h = stnums->h;
	rtextc.w = stextc->w;
	rtextc.h = stextc->h;
	rtextcc.w = stextcc->w;
	rtextcc.h = stextcc->h;
	rtexti.w = stexti->w;
	rtexti.h = stexti->h;
	rtextth.w = stextth->w;
	rtextth.h = stextth->h;
	rtextoh.w = stextoh->w;
	rtextoh.h = stextoh->h;
	rtextcu.w = stextcu->w;
	rtextcu.h = stextcu->h;
	rtextd.w = stextd->w;
	rtextd.h = stextd->h;
	SDL_FreeSurface(stnums);
	SDL_FreeSurface(stextc);
	SDL_FreeSurface(stextcc);
	SDL_FreeSurface(stexti);
	SDL_FreeSurface(stextth);
	SDL_FreeSurface(stextoh);
	SDL_FreeSurface(stextcu);
	SDL_FreeSurface(stextd);
	while (run)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT) run = false;
		}
		for (i = 0; i < BODYNUMS; i++)
		{
			if (crowd[i].state < 3) root.insert(&crowd[i]);
		}
		SDL_SetRenderDrawColor(ren, 68, 68, 68, 255);
		SDL_RenderClear(ren);
		for (i = 0; i < BODYNUMS; i++)
		{
			if (crowd[i].state < 3)
			{
				colld = root.retrieve(&crowd[i]);
				for (j = colld.begin(); j != colld.end(); ++j)
				{
					if (crowd[i].y + BODYSIZE > (*j)->y && crowd[i].y < (*j)->y + BODYSIZE && crowd[i].x + BODYSIZE > (*j)->x && crowd[i].x < (*j)->x + BODYSIZE)
					{
						if ((crowd[i].state == 1 || crowd[i].state == 2) && (*j)->state == 0)
						{
							(*j)->state = 2;
							(*j)->wait = INCUP;
							incuping++;
						}
						if (crowd[i].state == 0 && ((*j)->state == 1 || (*j)->state == 2))
						{
							crowd[i].state = 2;
							crowd[i].wait = INCUP;
							incuping++;
						}
					}
				}
			}
			if (crowd[i].state == 0)
			{
				if (crowd[i].x <= 0 || crowd[i].x >= BORDERX) crowd[i].dx = -crowd[i].dx;
				if (crowd[i].y <= 0 || crowd[i].y >= BORDERY) crowd[i].dy = -crowd[i].dy;
				crowd[i].x += crowd[i].dx;
				crowd[i].y += crowd[i].dy;
				renrct.x = crowd[i].x;
				renrct.y = crowd[i].y;
				SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
				SDL_RenderFillRect(ren, &renrct);
			}
			else if (crowd[i].state == 1)
			{
				if (crowd[i].wait == 0)
				{
					if (HOSPITAL > ohospital)
					{
						crowd[i].state = 3;
						crowd[i].wait = HEALTIME;
						infects--;
						ohospital++;
					}
					else
					{
						crowd[i].wait = HOSPITALRESPTIME;
						if (crowd[i].x <= 0 || crowd[i].x >= BORDERX) crowd[i].dx = -crowd[i].dx;
						if (crowd[i].y <= 0 || crowd[i].y >= BORDERY) crowd[i].dy = -crowd[i].dy;
						crowd[i].x += crowd[i].dx;
						crowd[i].y += crowd[i].dy;
						renrct.x = crowd[i].x;
						renrct.y = crowd[i].y;
						SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
						SDL_RenderFillRect(ren, &renrct);
						if (crowd[i].die == 0)
						{
							crowd[i].state = 4;
							infects--;
							death++;
						}
					}
				}
				else if (crowd[i].die == 0)
				{
					crowd[i].state = 4;
					infects--;
					death++;
				}
				else
				{
					if (crowd[i].x <= 0 || crowd[i].x >= BORDERX) crowd[i].dx = -crowd[i].dx;
					if (crowd[i].y <= 0 || crowd[i].y >= BORDERY) crowd[i].dy = -crowd[i].dy;
					crowd[i].x += crowd[i].dx;
					crowd[i].y += crowd[i].dy;
					renrct.x = crowd[i].x;
					renrct.y = crowd[i].y;
					SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
					SDL_RenderFillRect(ren, &renrct);
				}
				crowd[i].wait--;
				crowd[i].die--;
			}
			else if (crowd[i].state == 2)
			{
				if (crowd[i].wait == 0)
				{
					crowd[i].state = 1;
					crowd[i].wait = HOSPITALRESPTIME;
					crowd[i].die = DIETIME;
					incuping--;
					infects++;
					tinfects++;
					if (crowd[i].x <= 0 || crowd[i].x >= BORDERX) crowd[i].dx = -crowd[i].dx;
					if (crowd[i].y <= 0 || crowd[i].y >= BORDERY) crowd[i].dy = -crowd[i].dy;
					crowd[i].x += crowd[i].dx;
					crowd[i].y += crowd[i].dy;
					renrct.x = crowd[i].x;
					renrct.y = crowd[i].y;
					SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
					SDL_RenderFillRect(ren, &renrct);
				}
				else
				{
					if (crowd[i].x <= 0 || crowd[i].x >= BORDERX) crowd[i].dx = -crowd[i].dx;
					if (crowd[i].y <= 0 || crowd[i].y >= BORDERY) crowd[i].dy = -crowd[i].dy;
					crowd[i].x += crowd[i].dx;
					crowd[i].y += crowd[i].dy;
					renrct.x = crowd[i].x;
					renrct.y = crowd[i].y;
					SDL_SetRenderDrawColor(ren, 255, 255, 0, 255);
					SDL_RenderFillRect(ren, &renrct);
				}
				crowd[i].wait--;
			}
			else if (crowd[i].state == 3)
			{
				if (crowd[i].wait == 0)
				{
					crowd[i].state = 0;
					ohospital--;
					cure++;
				}
				crowd[i].wait--;
			}
		}
		root.clear();
		snumberc = TTF_RenderText_Solid(font, itoa(tinfects, numberc, 10), red);
		snumbercc = TTF_RenderText_Solid(font, itoa(infects, numbercc, 10), red);
		snumberi = TTF_RenderText_Solid(font, itoa(incuping, numberi, 10), yellow);
		snumberoh = TTF_RenderText_Solid(font, itoa(ohospital, numberoh, 10), white);
		snumbercu = TTF_RenderText_Solid(font, itoa(cure, numbercu, 10), green);
		snumberd = TTF_RenderText_Solid(font, itoa(death, numberd, 10), black);
		tnumberc = SDL_CreateTextureFromSurface(ren, snumberc);
		tnumbercc = SDL_CreateTextureFromSurface(ren, snumbercc);
		tnumberi = SDL_CreateTextureFromSurface(ren, snumberi);
		tnumberoh = SDL_CreateTextureFromSurface(ren, snumberoh);
		tnumbercu = SDL_CreateTextureFromSurface(ren, snumbercu);
		tnumberd = SDL_CreateTextureFromSurface(ren, snumberd);
		rnumberc.w = snumberc->w;
		rnumberc.h = snumberc->h;
		rnumbercc.w = snumbercc->w;
		rnumbercc.h = snumbercc->h;
		rnumberi.w = snumberi->w;
		rnumberi.h = snumberi->h;
		rnumberoh.w = snumberoh->w;
		rnumberoh.h = snumberoh->h;
		rnumbercu.w = snumbercu->w;
		rnumbercu.h = snumbercu->h;
		rnumberd.w = snumberd->w;
		rnumberd.h = snumberd->h;
		SDL_FreeSurface(snumberc);
		SDL_FreeSurface(snumbercc);
		SDL_FreeSurface(snumberi);
		SDL_FreeSurface(snumberoh);
		SDL_FreeSurface(snumbercu);
		SDL_FreeSurface(snumberd);
		SDL_RenderCopy(ren, ttnums, NULL, &rtnums);
		SDL_RenderCopy(ren, ttextc, NULL, &rtextc);
		SDL_RenderCopy(ren, ttextcc, NULL, &rtextcc);
		SDL_RenderCopy(ren, ttexti, NULL, &rtexti);
		SDL_RenderCopy(ren, ttextth, NULL, &rtextth);
		SDL_RenderCopy(ren, ttextoh, NULL, &rtextoh);
		SDL_RenderCopy(ren, ttextcu, NULL, &rtextcu);
		SDL_RenderCopy(ren, ttextd, NULL, &rtextd);
		SDL_RenderCopy(ren, tnumberc, NULL, &rnumberc);
		SDL_RenderCopy(ren, tnumbercc, NULL, &rnumbercc);
		SDL_RenderCopy(ren, tnumberi, NULL, &rnumberi);
		SDL_RenderCopy(ren, tnumberoh, NULL, &rnumberoh);
		SDL_RenderCopy(ren, tnumbercu, NULL, &rnumbercu);
		SDL_RenderCopy(ren, tnumberd, NULL, &rnumberd);
		SDL_RenderPresent(ren);
		SDL_DestroyTexture(tnumberc);
		SDL_DestroyTexture(tnumbercc);
		SDL_DestroyTexture(tnumberi);
		SDL_DestroyTexture(tnumberoh);
		SDL_DestroyTexture(tnumbercu);
		SDL_DestroyTexture(tnumberd);
	}
	SDL_DestroyTexture(ttnums);
	SDL_DestroyTexture(ttextc);
	SDL_DestroyTexture(ttextcc);
	SDL_DestroyTexture(ttexti);
	SDL_DestroyTexture(ttextth);
	SDL_DestroyTexture(ttextoh);
	SDL_DestroyTexture(ttextcu);
	SDL_DestroyTexture(ttextd);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(wnd);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
	return 0;
}
