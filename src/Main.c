//#include "C:/Wichtig/System/Static/Library/WindowEngine.h"
#include "/home/codeleaded/System/Static/Library/WindowEngine.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"
#include "/home/codeleaded/System/Static/Library/WavePropagation.h"

typedef struct WorldPoint2D {
	float x;
	float y;
	int t;
} WorldPoint2D;

#define MAP_WIDTH	16
#define MAP_HEIGHT	16

WavePropagationNode* nodes;
WavePropagationNode* start;
WavePropagationNode* end;
TransformedView tv;


void WavePropagationNode_UpdateNeighbours(){
	for(int i = 0;i<MAP_HEIGHT;i++){
		for(int j = 0;j<MAP_WIDTH;j++){
			WavePropagationNode* an = nodes + (i * MAP_WIDTH + j);
			Vector_Clear(&an->neighbours);
			
			// immediate neighbours
			if(i>0){
				WavePropagationNode* nb = nodes + ((i - 1) * MAP_WIDTH + j);
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(i<MAP_HEIGHT - 1){
				WavePropagationNode* nb = nodes + ((i + 1) * MAP_WIDTH + j);
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(j>0){
				WavePropagationNode* nb = nodes + (i * MAP_WIDTH + (j - 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(j<MAP_WIDTH - 1){
				WavePropagationNode* nb = nodes + (i * MAP_WIDTH + (j + 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}

			// cornor neighbours
			if(i>0 && j>0){
				WavePropagationNode* nb = nodes + ((i - 1) * MAP_WIDTH + (j - 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(i>0 && j<MAP_WIDTH - 1){
				WavePropagationNode* nb = nodes + ((i - 1) * MAP_WIDTH + (j + 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(i<MAP_HEIGHT - 1 && j>0){
				WavePropagationNode* nb = nodes + ((i + 1) * MAP_WIDTH + (j - 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
			if(i<MAP_HEIGHT - 1 && j<MAP_WIDTH - 1){
				WavePropagationNode* nb = nodes + ((i + 1) * MAP_WIDTH + (j + 1));
				if(!((WorldPoint2D*)nb->data)->t) Vector_Push(&an->neighbours,(WavePropagationNode*[]){ nb });
			}
		}
	}
}

void Setup(AlxWindow* w){
	tv = TransformedView_New((Vec2){ GetWidth(),GetHeight() });

	nodes = (WavePropagationNode*)malloc(sizeof(WavePropagationNode) * MAP_WIDTH * MAP_HEIGHT);
	for(int i = 0;i<MAP_HEIGHT;i++){
		for(int j = 0;j<MAP_WIDTH;j++){
			nodes[i * MAP_WIDTH + j] = WavePropagationNode_New((WorldPoint2D[]){ { j,i,0 } },sizeof(WorldPoint2D));
		}
	}

	start = NULL;
	end = NULL;
}
void Update(AlxWindow* w){
	TransformedView_Output(&tv,(Vec2){ GetWidth(),GetHeight() });
	TransformedView_HandlePanZoom(&tv,window.Strokes,GetMouse());
	Rect r = TransformedView_ScreenWorldRect(&tv,GetScreenRect());

	if(Stroke(ALX_MOUSE_L).PRESSED){
		Vec2 m = TransformedView_ScreenWorldPos(&tv,GetMouse());
		if(m.x>=0 && m.x<MAP_WIDTH && m.y>=0 && m.y<MAP_HEIGHT){
			WavePropagationNode* an = nodes + ((int)m.y * MAP_WIDTH + (int)m.x);
			if(Stroke(ALX_KEY_W).DOWN){
				start = an;
			}else if(Stroke(ALX_KEY_S).DOWN){
				end = an;
			}else{
				WorldPoint2D* wp = (WorldPoint2D*)an->data;
				wp->t = !wp->t;
			}	
		}
		else if(Stroke(ALX_KEY_W).DOWN) start = NULL;
		else if(Stroke(ALX_KEY_S).DOWN) end = NULL;

		WavePropagationNode_UpdateNeighbours();
		
		if(end){
			// for(int i = 0;i<MAP_WIDTH * MAP_HEIGHT;i++){
			// 	WavePropagationNode* an = nodes + i;
			// 	an->visited = 0;
			// 	an->distance = INFINITY;
			// }
			WavePropagationNode_ResetND(end,2);
			WavePropagationNode_UpdateND(end,2);
		}
	}
	
	Clear(BLACK);

	for(int i = 0;i<MAP_HEIGHT;i++){
		for(int j = 0;j<MAP_WIDTH;j++){
			WavePropagationNode* an = nodes + (i * MAP_WIDTH + j);
			WorldPoint2D* awp = (WorldPoint2D*)an->data;
			
			for(int k = 0;k<an->neighbours.size;k++){
				WavePropagationNode* nan = *(WavePropagationNode**)Vector_Get(&an->neighbours,k);
				WorldPoint2D* nwp = (WorldPoint2D*)nan->data;

				Vec2 ap = TransformedView_WorldScreenPos(&tv,(Vec2){ awp->x + 0.5f,awp->y + 0.5f });
				Vec2 np = TransformedView_WorldScreenPos(&tv,(Vec2){ nwp->x + 0.5f,nwp->y + 0.5f });
				RenderLine(ap,np,BLUE,1.0f);
			}
		}
	}

	for(int i = 0;i<MAP_HEIGHT;i++){
		for(int j = 0;j<MAP_WIDTH;j++){
			WavePropagationNode* an = nodes + (i * MAP_WIDTH + j);
			WorldPoint2D* wp = (WorldPoint2D*)an->data;
			
			Vec2 p = TransformedView_WorldScreenPos(&tv,(Vec2){ wp->x + 0.05f,wp->y + 0.05f });
			Vec2 d = TransformedView_WorldScreenLength(&tv,(Vec2){ 0.9f,0.9f });
			RenderRect(p.x,p.y,d.x,d.y,an == start ? GREEN : (an == end ? RED : (wp->t ? WHITE : (an->visited ? LIGHT_BLUE : BLUE))));
		
			if(!wp->t){
				WavePropagationNode* nan = WavePropagationNode_FindNearestND(an);
				if(nan){
					WorldPoint2D* nwp = (WorldPoint2D*)nan->data;

					Vec2 dir = Vec2_Mulf(Vec2_Norm(Vec2_Sub((Vec2){ nwp->x,nwp->y },(Vec2){ wp->x,wp->y })),0.45f);
					Vec2 ap = TransformedView_WorldScreenPos(&tv,(Vec2){ wp->x + 0.5f,wp->y + 0.5f });
					Vec2 pp = TransformedView_WorldScreenPos(&tv,(Vec2){ wp->x + 0.5f + dir.x,wp->y + 0.5f + dir.y });

					// Vec2 ap = TransformedView_WorldScreenPos(&tv,(Vec2){ wp->x + 0.5f,wp->y + 0.5f });
					// Vec2 pp = TransformedView_WorldScreenPos(&tv,(Vec2){ nwp->x + 0.5f,nwp->y + 0.5f });
					RenderLine(ap,pp,MAGENTA,1.0f);
				}
			}
		}
	}

	if(start && end){
		WavePropagationNode* p = start;
		WavePropagationNode* next = start;
		int updates = 0;
		while(p != end){
			for(int k = 0;k<p->neighbours.size;k++){
				WavePropagationNode* nan = *(WavePropagationNode**)Vector_Get(&p->neighbours,k);
				if(nan->distance < next->distance){
					next = nan;
					updates++;
				}
			}

			if(updates==0) break;

			WorldPoint2D* awp = (WorldPoint2D*)p->data;
			WorldPoint2D* pwp = (WorldPoint2D*)next->data;

			Vec2 ap = TransformedView_WorldScreenPos(&tv,(Vec2){ awp->x + 0.5f,awp->y + 0.5f });
			Vec2 pp = TransformedView_WorldScreenPos(&tv,(Vec2){ pwp->x + 0.5f,pwp->y + 0.5f });
			RenderLine(ap,pp,YELLOW,1.0f);

			p = next;
			updates = 0;
		}
	}
	
	//String str = String_Format("MI:%d",nMaxIterations);
	//CStr_RenderSizeAlxFont(WINDOW_STD_ARGS,&window.font,str.Memory,str.size,0.0f,0.0f,WHITE);
	//String_Free(&str);
}
void Delete(AlxWindow* w){
	for(int i = 0;i<MAP_WIDTH * MAP_HEIGHT;i++){
		WavePropagationNode_Free(nodes + i);
	}
	
	if(nodes) free(nodes);
	nodes = NULL;
}

int main(){
    if(Create("Wave Propagation",1920,1080,1,1,Setup,Update,Delete))
        Start();
    return 0;
}