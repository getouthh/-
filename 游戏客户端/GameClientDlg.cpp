#include "Stdafx.h"
#include "GameOption.h"
#include "GameClient.h"
#include "GameClientDlg.h"

//////////////////////////////////////////////////////////////////////////
//宏定义

//游戏定时器
#define IDI_OUT_CARD					200								//出牌定时器
#define IDI_MOST_CARD					201								//最大定时器
#define IDI_START_GAME					202								//开始定时器
#define IDI_LAND_SCORE					203								//叫分定时器

//////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CGameClientDlg, CGameFrameDlg)
	ON_WM_TIMER()
	ON_MESSAGE(IDM_START,OnStart)
	ON_MESSAGE(IDM_OUT_CARD,OnOutCard)
	ON_MESSAGE(IDM_PASS_CARD,OnPassCard)
	ON_MESSAGE(IDM_LAND_SCORE,OnLandScore)
	ON_MESSAGE(IDM_AUTO_OUTCARD,OnAutoOutCard)
	ON_MESSAGE(IDM_LEFT_HIT_CARD,OnLeftHitCard)
	ON_MESSAGE(IDM_RIGHT_HIT_CARD,OnRightHitCard)
END_MESSAGE_MAP()

//////////////////////////////////////////////////////////////////////////

//构造函数
CGameClientDlg::CGameClientDlg() : CGameFrameDlg(&m_GameClientView)
{
	//游戏变量
	m_wBombTime=1;
	m_bHandCardCount=0;
	m_bMagicValue=0;
	m_bMagicCard=0;
	m_bHintCardCount=0;
	m_wLandUser=INVALID_CHAIR;
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bHandCardData,0,sizeof(m_bHandCardData));

	//配置变量
	m_bDeasilOrder=false;
	m_dwCardHSpace=DEFAULT_PELS;

	//出牌变量
	m_bTurnCardCount=0;
	m_bTurnOutType=CT_INVALID;
	memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));
	memset(m_bHintCardData,0,sizeof(m_bHintCardData));
	memset(m_bHandCardData,0,sizeof(m_bTempHandCard));

	//辅助变量
	m_wTimeOutCount=0;
	m_wMostUser=INVALID_CHAIR;

	return;
}

//析构函数
CGameClientDlg::~CGameClientDlg()
{
}

//初始函数
bool CGameClientDlg::InitGameFrame()
{
	//设置标题
	SetWindowText(TEXT("赖子斗地主(1张赖子)  --  Ver：6.0.1.0"));

	//设置图标
	HICON hIcon=LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon,TRUE);
	SetIcon(hIcon,FALSE);

	//读取配置
	m_dwCardHSpace=AfxGetApp()->GetProfileInt(TEXT("GameOption"),TEXT("CardSpace"),DEFAULT_PELS);
	m_bDeasilOrder=AfxGetApp()->GetProfileInt(TEXT("GameOption"),TEXT("DeasilOrder"),FALSE)?true:false;

	//调整参数
	if ((m_dwCardHSpace>MAX_PELS)||(m_dwCardHSpace<LESS_PELS)) m_dwCardHSpace=DEFAULT_PELS;

	//配置控件
	m_GameClientView.SetUserOrder(m_bDeasilOrder);
	m_GameClientView.m_HandCardControl.SetCardSpace(m_dwCardHSpace,0,20);
//	m_GameClientView.m_btStart.ShowWindow( SW_SHOW);

	//test
#ifdef DEBUG_GAME

	BYTE bData[21]={67,2,28,56,55,20,7,59,39,8,24,40,25,9,10,26,11,27,43,29,12};

	m_bHandCardCount=21;
	
	m_GameLogic.SortCardList(bData,m_bHandCardCount);
	CopyMemory(m_bHandCardData,bData,m_bHandCardCount);
	m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
	BYTE bOutData[5]={10,26,42,9,25};
	m_bTurnCardCount=sizeof(bOutData);
	m_GameLogic.SortCardList(bOutData,m_bTurnCardCount);
	CopyMemory(m_bTurnCardData,bOutData,m_bTurnCardCount);
	m_bTurnOutType=m_GameLogic.GetCardType(m_bTurnCardData,m_bTurnCardCount);
	//char *str=new char[100];
	//sprintf(str,"%d",m_bTurnCardCount);
	//MessageBox(str);
	//delete []str;
	ActiveGameFrame();
	m_GameClientView.m_btOutCard.EnableWindow(FALSE);
	m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
	m_GameClientView.m_btPassCard.EnableWindow(FALSE);
	m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
	m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
	m_GameClientView.m_btAutoOutCard.EnableWindow(TRUE);	
	m_GameClientView.m_HandCardControl.SetPositively(true);
	m_GameClientView.m_HandCardControl.SetDisplayFlag(true);
#endif
	return true;
}

//重置框架
void CGameClientDlg::ResetGameFrame()
{
	//游戏变量
	m_wBombTime=1;
	m_bHandCardCount=0;
	m_bHintCardCount=0;
	m_bMagicValue=0;
	m_bMagicCard=0;
	m_wLandUser=INVALID_CHAIR;
	memset(m_bCardCount,0,sizeof(m_bCardCount));
	memset(m_bHandCardData,0,sizeof(m_bHandCardData));

	//出牌变量
	m_bTurnCardCount=0;
	m_bTurnOutType=CT_INVALID;
	memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));
	memset(m_bHandCardData,0,sizeof(m_bHintCardData));
	memset(m_bTempHandCard,0,sizeof(m_bTempHandCard));

	//辅助变量
	m_wTimeOutCount=0;
	m_wMostUser=INVALID_CHAIR;

	//删除定时
	KillGameTimer(0);
	KillTimer(IDI_MOST_CARD);

	return;
}

//游戏设置
void CGameClientDlg::OnGameOptionSet()
{
	//构造数据
	CGameOption GameOption;
	GameOption.m_dwCardHSpace=m_dwCardHSpace;
	GameOption.m_bEnableSound=IsEnableSound();
	GameOption.m_bDeasilOrder=m_GameClientView.IsDeasilOrder();

	//配置数据
	if (GameOption.DoModal()==IDOK)
	{
		//获取参数
		m_bDeasilOrder=GameOption.m_bDeasilOrder;
		m_dwCardHSpace=GameOption.m_dwCardHSpace;

		//设置控件
		EnableSound(GameOption.m_bEnableSound);
		m_GameClientView.SetUserOrder(GameOption.m_bDeasilOrder);
		m_GameClientView.m_HandCardControl.SetCardSpace(m_dwCardHSpace,0,20);

		//保存配置
		AfxGetApp()->WriteProfileInt(TEXT("GameOption"),TEXT("CardSpace"),m_dwCardHSpace);
		AfxGetApp()->WriteProfileInt(TEXT("GameOption"),TEXT("DeasilOrder"),m_bDeasilOrder?TRUE:FALSE);
	}

	return;
}

//时间消息
bool CGameClientDlg::OnTimerMessage(WORD wChairID, UINT nElapse, UINT nTimerID)
{
	switch (nTimerID)
	{
	case IDI_OUT_CARD:			//用户出牌
		{
			//超时判断
			if (nElapse==0)
			{
				if ((IsLookonMode()==false)&&(wChairID==GetMeChairID())) AutomatismOutCard();
				return false;
			}

			//播放声音
			if (m_bHandCardCount<m_bTurnCardCount) return true;
			if ((nElapse<=10)&&(wChairID==GetMeChairID())&&(IsLookonMode()==false)) PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));

			return true;
		}
	case IDI_START_GAME:		//开始游戏
		{
			if (nElapse==0)
			{
				if ((IsLookonMode()==false)&&(wChairID==GetMeChairID())) OnStart(0,0);
				return false;
			}
			if ((nElapse<=10)&&(wChairID==GetMeChairID())&&(IsLookonMode()==false)) PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));

			return true;
		}
	case IDI_LAND_SCORE:		//挖坑叫分
		{
			if (nElapse==0)
			{
				if ((IsLookonMode()==false)&&(wChairID==GetMeChairID())) OnLandScore(255,255);
				return false;
			}
			if ((nElapse<=10)&&(wChairID==GetMeChairID())&&(IsLookonMode()==false)) PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WARN"));

			return true;
		}
	}

	return false;
}

//旁观状态
void CGameClientDlg::OnLookonChanged(bool bLookonUser, const void * pBuffer, WORD wDataSize)
{
}

//网络消息
bool CGameClientDlg::OnGameMessage(WORD wSubCmdID, const void * pBuffer, WORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_SEND_CARD:		//发送扑克
		{
			return OnSubSendCard(pBuffer,wDataSize);
		}
	case SUB_S_LAND_SCORE:	//用户叫分
		{
			return OnSubLandScore(pBuffer,wDataSize);
		}
	case SUB_S_GAME_START:		//游戏开始
		{
			return OnSubGameStart(pBuffer,wDataSize);
		}
	case SUB_S_OUT_CARD:		//用户出牌
		{
			return OnSubOutCard(pBuffer,wDataSize);
		}
	case SUB_S_PASS_CARD:		//放弃出牌
		{
			return OnSubPassCard(pBuffer,wDataSize);
		}
	case SUB_S_GAME_END:		//游戏结束
		{
			return OnSubGameEnd(pBuffer,wDataSize);
		}
	}

	return false;
}

//游戏场景
bool CGameClientDlg::OnGameSceneMessage(BYTE cbGameStation, bool bLookonOther, const void * pBuffer, WORD wDataSize)
{
	switch (cbGameStation)
	{
	case GS_WK_FREE:	//空闲状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusFree)) return false;
			CMD_S_StatusFree * pStatusFree=(CMD_S_StatusFree *)pBuffer;

			//设置界面
			m_GameClientView.SetBaseScore(pStatusFree->lBaseScore);
			m_GameClientView.m_btStart.ShowWindow( SW_SHOW);

			//设置控件
			if (IsLookonMode()==false)
			{
				m_GameClientView.m_btStart.ShowWindow(TRUE);
				m_GameClientView.m_btStart.SetFocus();
			}

			//设置扑克
			if (IsLookonMode()==false) m_GameClientView.m_HandCardControl.SetDisplayFlag(true);

			return true;
		}
	case GS_WK_SCORE:	//叫分状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusScore)) return false;
			CMD_S_StatusScore * pStatusScore=(CMD_S_StatusScore *)pBuffer;

			//设置变量
			m_bHandCardCount=17;
			for (WORD i=0;i<GAME_PLAYER;i++) m_bCardCount[i]=17;
			CopyMemory(m_bHandCardData,pStatusScore->bCardData,m_bHandCardCount);

			//设置界面
			for (WORD i=0;i<GAME_PLAYER;i++)	
			{
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.SetCardCount(wViewChairID,m_bCardCount[i]);
				m_GameClientView.SetLandScore(wViewChairID,pStatusScore->bScoreInfo[i]);
			}
			m_GameClientView.ShowLandTitle(true);
			m_GameClientView.SetBaseScore(pStatusScore->lBaseScore);

			//按钮控制
			if ((IsLookonMode()==false)&&(pStatusScore->wCurrentUser==GetMeChairID()))
			{
				m_GameClientView.m_btGiveUpScore.ShowWindow(SW_SHOW);
				m_GameClientView.m_btOneScore.ShowWindow(pStatusScore->bLandScore==0?SW_SHOW:SW_HIDE);
				m_GameClientView.m_btTwoScore.ShowWindow(pStatusScore->bLandScore<=1?SW_SHOW:SW_HIDE);
				m_GameClientView.m_btThreeScore.ShowWindow(pStatusScore->bLandScore<=2?SW_SHOW:SW_HIDE);
			}

			//设置扑克
			BYTE bCardData[4]={0,0,0,0};
			m_GameClientView.m_BackCardControl.SetCardData(bCardData,4);
			m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
			if (IsLookonMode()==false) m_GameClientView.m_HandCardControl.SetDisplayFlag(true);

			//设置时间
			SetGameTimer(pStatusScore->wCurrentUser,IDI_LAND_SCORE,30);

			return true;
		}
	case GS_WK_PLAYING:		//游戏状态
		{
			//效验数据
			if (wDataSize!=sizeof(CMD_S_StatusPlay)) return false;
			CMD_S_StatusPlay * pStatusPlay=(CMD_S_StatusPlay *)pBuffer;

			//设置变量
			m_bTurnCardCount=pStatusPlay->bTurnCardCount;
			m_bHandCardCount=pStatusPlay->bCardCount[GetMeChairID()];
			m_bTurnOutType=m_GameLogic.GetCardType(pStatusPlay->bTurnCardData,m_bTurnCardCount);
			CopyMemory(m_bHandCardData,pStatusPlay->bCardData,m_bHandCardCount);
			CopyMemory(m_bTurnCardData,pStatusPlay->bTurnCardData,pStatusPlay->bTurnCardCount);

			//设置界面
			for (BYTE i=0;i<GAME_PLAYER;i++)
			{
				WORD wViewChairID=SwitchViewChairID(i);
				m_bCardCount[i]=pStatusPlay->bCardCount[i];
				m_GameClientView.SetCardCount(wViewChairID,pStatusPlay->bCardCount[i]);
			}
			m_GameClientView.SetBombTime(pStatusPlay->wBombTime);
			m_GameClientView.SetBaseScore(pStatusPlay->lBaseScore);
			m_GameClientView.m_BackCardControl.SetCardData(pStatusPlay->bBackCard,4);
			m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
			m_GameClientView.SetLandUser(SwitchViewChairID(pStatusPlay->wLandUser),pStatusPlay->bLandScore);

			//玩家设置
			if ((IsLookonMode()==false)&&(pStatusPlay->wCurrentUser==GetMeChairID()))
			{
				m_GameClientView.m_btOutCard.EnableWindow(FALSE);
				m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
				m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
				m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
				m_GameClientView.m_btPassCard.EnableWindow((m_bTurnCardCount!=0)?TRUE:FALSE);
				m_GameClientView.m_btAutoOutCard.EnableWindow((m_bTurnCardCount!=0)?TRUE:FALSE);
			}

			//桌面设置
			if (m_bTurnCardCount!=0)
			{
				WORD wViewChairID=SwitchViewChairID(pStatusPlay->wLastOutUser);
				m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(m_bTurnCardData,m_bTurnCardCount);
			}

			//设置定时器
			SetGameTimer(pStatusPlay->wCurrentUser,IDI_OUT_CARD,30);

			//设置扑克
			if (IsLookonMode()==false) 
			{
				m_GameClientView.m_HandCardControl.SetPositively(true);
				m_GameClientView.m_HandCardControl.SetDisplayFlag(true);
			}

			return true;
		}
	}

	return false;
}

//发送扑克
bool CGameClientDlg::OnSubSendCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_SendCard));
	if (wDataSize!=sizeof(CMD_S_SendCard)) return false;

	//变量定义
	CMD_S_SendCard * pSendCard=(CMD_S_SendCard *)pBuffer;

	//设置数据
	m_bHandCardCount=CountArray(pSendCard->bCardData);
	CopyMemory(m_bHandCardData,pSendCard->bCardData,sizeof(pSendCard->bCardData));
	for (WORD i=0;i<GAME_PLAYER;i++) m_bCardCount[i]=CountArray(pSendCard->bCardData);

	//设置界面
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		m_GameClientView.SetLandScore(i,0);
		m_GameClientView.SetPassFlag(i,false);
		m_GameClientView.SetCardCount(i,m_bCardCount[i]);
		m_GameClientView.m_UserCardControl[i].SetCardData(NULL,0);
	}
	if (IsLookonMode()==true)
	{
		m_GameClientView.SetLandUser(INVALID_CHAIR,0);
		m_GameClientView.m_ScoreView.ShowWindow(SW_HIDE);
		m_GameClientView.m_LeaveCardControl[0].SetCardData(NULL,0);
		m_GameClientView.m_LeaveCardControl[1].SetCardData(NULL,0);
	}
	m_GameClientView.ShowLandTitle(true);
	m_GameClientView.SetBombTime(m_wBombTime);

	//设置扑克
	BYTE bBackCard[4]={0,0,0,0};
	m_GameClientView.m_HandCardControl.SetCardData(pSendCard->bCardData,17);
	m_GameClientView.m_BackCardControl.SetCardData(bBackCard,CountArray(bBackCard));
	if (IsLookonMode()==true) m_GameClientView.m_HandCardControl.SetDisplayFlag(false);

	//当前玩家
	if ((IsLookonMode()==false)&&(pSendCard->wCurrentUser==GetMeChairID()))
	{
		ActiveGameFrame();
		m_GameClientView.m_btOneScore.ShowWindow(SW_SHOW);
		m_GameClientView.m_btTwoScore.ShowWindow(SW_SHOW);
		m_GameClientView.m_btThreeScore.ShowWindow(SW_SHOW);
		m_GameClientView.m_btGiveUpScore.ShowWindow(SW_SHOW);
	}

	//播放声音
	PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_START"));

	//设置时间
	SetGameTimer(pSendCard->wCurrentUser,IDI_LAND_SCORE,30);

	return true;
}

//用户叫分
bool CGameClientDlg::OnSubLandScore(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_LandScore));
	if (wDataSize!=sizeof(CMD_S_LandScore)) return false;

	//消息处理
	CMD_S_LandScore * pLandScore=(CMD_S_LandScore *)pBuffer;

	//设置界面
	WORD wViewChairID=SwitchViewChairID(pLandScore->bLandUser);
	m_GameClientView.SetLandScore(wViewChairID,pLandScore->bLandScore);

	//玩家设置
	if ((IsLookonMode()==false)&&(pLandScore->wCurrentUser==GetMeChairID()))
	{
		ActiveGameFrame();
		m_GameClientView.m_btGiveUpScore.ShowWindow(SW_SHOW);
		m_GameClientView.m_btOneScore.ShowWindow(pLandScore->bCurrentScore==0?SW_SHOW:SW_HIDE);
		m_GameClientView.m_btTwoScore.ShowWindow(pLandScore->bCurrentScore<=1?SW_SHOW:SW_HIDE);
		m_GameClientView.m_btThreeScore.ShowWindow(pLandScore->bCurrentScore<=2?SW_SHOW:SW_HIDE);
	}

	//播放声音
	PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));

	//设置时间
	SetGameTimer(pLandScore->wCurrentUser,IDI_LAND_SCORE,30);

	return true;
}

//游戏开始
bool CGameClientDlg::OnSubGameStart(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_GameStart));
	if (wDataSize!=sizeof(CMD_S_GameStart)) return false;

	//消息处理
	CMD_S_GameStart * pGameStart=(CMD_S_GameStart *)pBuffer;

	//设置变量
	m_wBombTime=1;
	m_bTurnCardCount=0;
	m_bTurnOutType=CT_INVALID;
	m_wLandUser=pGameStart->wLandUser;
	m_bCardCount[pGameStart->wLandUser]=21;
	ZeroMemory(m_bTurnCardData,sizeof(m_bTurnCardData));

	//设置控件
	m_GameClientView.ShowLandTitle(false);
	m_GameClientView.m_BackCardControl.SetCardData(pGameStart->bBackCard,CountArray(pGameStart->bBackCard));

	//设置界面
	m_GameClientView.SetLandScore(INVALID_CHAIR,0);
	m_GameClientView.SetCardCount(SwitchViewChairID(pGameStart->wLandUser),m_bCardCount[pGameStart->wLandUser]);

	//地主设置
	if (pGameStart->wLandUser==GetMeChairID())
	{
		BYTE bCardCound=m_bHandCardCount;
		m_bHandCardCount+=CountArray(pGameStart->bBackCard);
		CopyMemory(&m_bHandCardData[bCardCound],pGameStart->bBackCard,sizeof(pGameStart->bBackCard));
		m_GameLogic.SortCardList(m_bHandCardData,m_bHandCardCount);
		m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
	}
	m_GameClientView.SetLandUser(SwitchViewChairID(pGameStart->wLandUser),pGameStart->bLandScore);

	//玩家设置
	if (IsLookonMode()==false) m_GameClientView.m_HandCardControl.SetPositively(true);

	//当前玩家
	if ((IsLookonMode()==false)&&(pGameStart->wCurrentUser==GetMeChairID()))
	{
		ActiveGameFrame();
		m_GameClientView.m_btOutCard.EnableWindow(FALSE);
		m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btPassCard.EnableWindow(FALSE);
		m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btAutoOutCard.EnableWindow(FALSE);
	}	
	m_GameClientView.m_LeaveCardControl[0].SetBackCard(m_bCardCount[0]);//.SetCardData(m_GameClientView.m_byZerodata,m_bCardCount[0]);
	m_GameClientView.m_LeaveCardControl[1].SetBackCard(m_bCardCount[1]);//.SetCardData(m_GameClientView.m_byZerodata,m_bCardCount[1]);
	//播放声音
	PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_START"));

	//设置时间
	SetGameTimer(pGameStart->wCurrentUser,IDI_OUT_CARD,30);

	return true;
}

//用户出牌
bool CGameClientDlg::OnSubOutCard(const void * pBuffer, WORD wDataSize)
{
	//变量定义
	CMD_S_OutCard * pOutCard=(CMD_S_OutCard *)pBuffer;
	WORD wHeadSize=sizeof(CMD_S_OutCard)-sizeof(pOutCard->bCardData);

	//效验数据
	ASSERT(wDataSize>=wHeadSize);
	if (wDataSize<wHeadSize) return false;
	ASSERT(wDataSize==(wHeadSize+pOutCard->bCardCount*sizeof(pOutCard->bCardData[0])));
	if (wDataSize!=(wHeadSize+pOutCard->bCardCount*sizeof(pOutCard->bCardData[0]))) return false;

	//删除定时器
	KillTimer(IDI_MOST_CARD);
	KillGameTimer(IDI_OUT_CARD);

	//界面设置
	WORD wOutViewChairID=SwitchViewChairID(pOutCard->wOutCardUser);
	m_bCardCount[pOutCard->wOutCardUser]-=pOutCard->bCardCount;
	m_GameClientView.SetCardCount(wOutViewChairID,m_bCardCount[pOutCard->wOutCardUser]);
	if( GetMeChairID()!=pOutCard->wOutCardUser)
	{
		if(wOutViewChairID==0)
			m_GameClientView.m_LeaveCardControl[0].SetBackCard(m_bCardCount[pOutCard->wOutCardUser]);
		if(wOutViewChairID==2)
			m_GameClientView.m_LeaveCardControl[1].SetBackCard(m_bCardCount[pOutCard->wOutCardUser]);
	}
	//出牌设置
	if ((IsLookonMode()==true)||(GetMeChairID()!=pOutCard->wOutCardUser))
	{
		m_GameClientView.m_UserCardControl[wOutViewChairID].SetCardData(pOutCard->bCardData,pOutCard->bCardCount);
	}

	//清理桌面
	if (m_bTurnCardCount==0)
	{
		for (WORD i=0;i<GAME_PLAYER;i++) 
		{
			if (i==pOutCard->wOutCardUser) continue;
			m_GameClientView.m_UserCardControl[SwitchViewChairID(i)].SetCardData(NULL,0);
		}
		m_GameClientView.SetPassFlag(INVALID_CHAIR,false);
	}

	//记录出牌
	m_bTurnCardCount=pOutCard->bCardCount;
	if(m_GameLogic.IsHadRoguishCard(pOutCard->bCardData,pOutCard->bCardCount))
	{
		m_bTurnOutType=m_GameLogic.GetMagicCardType(pOutCard->bCardData,pOutCard->bCardCount,&m_bMagicCard);
		//炸弹判断
		if (m_bTurnOutType==CT_BOMB_SOFT)
		{
			m_wBombTime*=2;
			m_GameClientView.SetBombTime(m_wBombTime);
		}
		CopyMemory(m_bTurnCardData,pOutCard->bCardData,pOutCard->bCardCount);
		m_bTurnCardData[0]=m_bMagicCard;
		m_GameLogic.SortCardList(m_bTurnCardData,m_bTurnCardCount);
	}
	else
	{	
		m_bTurnOutType=m_GameLogic.GetCardType(pOutCard->bCardData,pOutCard->bCardCount);
		//炸弹判断
		if ((m_bTurnOutType==CT_BOMB_CARD)||(m_bTurnOutType==CT_MISSILE_CARD))
		{
			m_wBombTime*=4;
			m_GameClientView.SetBombTime(m_wBombTime);
		}
		CopyMemory(m_bTurnCardData,pOutCard->bCardData,pOutCard->bCardCount);
	}

	//自己扑克
	if ((IsLookonMode()==true)&&(pOutCard->wOutCardUser==GetMeChairID()))
	{
		//删除扑克 
		BYTE bSourceCount=m_bHandCardCount;
		m_bHandCardCount-=pOutCard->bCardCount;
		m_GameLogic.RemoveCard(pOutCard->bCardData,pOutCard->bCardCount,m_bHandCardData,bSourceCount);

		//设置界面
		m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
	}

	//最大判断
	if (pOutCard->wCurrentUser==pOutCard->wOutCardUser)
	{
		//设置变量
		m_bTurnCardCount=0;
		m_bTurnOutType=CT_INVALID;
		m_wMostUser=pOutCard->wCurrentUser;
		memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));

		//设置界面
		for (WORD i=0;i<GAME_PLAYER;i++)
		{
			if (i!=pOutCard->wOutCardUser)
			{
				WORD wViewChairID=SwitchViewChairID(i);
				m_GameClientView.SetPassFlag(wViewChairID,true);
				m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(NULL,0);
			}
		}

		//播放声音
		PlayGameSound(AfxGetInstanceHandle(),TEXT("MOST_CARD"));

		//设置定时器
		SetTimer(IDI_MOST_CARD,3000,NULL);

		return true;
	}

	//播放声音
	if ((IsLookonMode()==true)||(GetMeChairID()!=pOutCard->wOutCardUser)) PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));

	//玩家设置
	if (pOutCard->wCurrentUser!=INVALID_CHAIR)
	{
		WORD wViewChairID=SwitchViewChairID(pOutCard->wCurrentUser);
		m_GameClientView.SetPassFlag(wViewChairID,false);
		m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(NULL,0);
	}

	//玩家设置
	if ((IsLookonMode()==false)&&(pOutCard->wCurrentUser==GetMeChairID()))
	{
		ActiveGameFrame();
		m_GameClientView.m_btPassCard.EnableWindow(TRUE);
		m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btAutoOutCard.EnableWindow(TRUE);
		m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btOutCard.EnableWindow((VerdictOutCard()==true)?TRUE:FALSE);
	}

	//设置时间
	if (pOutCard->wCurrentUser!=INVALID_CHAIR)
	{
		BYTE bCardCount=m_bCardCount[pOutCard->wCurrentUser];
		SetGameTimer(pOutCard->wCurrentUser,IDI_OUT_CARD,(bCardCount<m_bTurnCardCount)?3:30);
	}

	return true;
}

//放弃出牌
bool CGameClientDlg::OnSubPassCard(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	if (wDataSize!=sizeof(CMD_S_PassCard)) return false;
	CMD_S_PassCard * pPassCard=(CMD_S_PassCard *)pBuffer;

	//删除定时器
	KillGameTimer(IDI_OUT_CARD);

	//玩家设置
	if ((IsLookonMode()==true)||(pPassCard->wPassUser!=GetMeChairID()))
	{
		WORD wViewChairID=SwitchViewChairID(pPassCard->wPassUser);
		m_GameClientView.SetPassFlag(wViewChairID,true);
		m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(NULL,0);
	}

	//一轮判断
	if (pPassCard->bNewTurn==TRUE)
	{
		m_bTurnCardCount=0;
		m_bTurnOutType=CT_INVALID;
		memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));
	}

	//设置界面
	WORD wViewChairID=SwitchViewChairID(pPassCard->wCurrentUser);
	m_GameClientView.SetPassFlag(wViewChairID,false);
	m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(NULL,0);

	//玩家设置
	if ((IsLookonMode()==false)&&(pPassCard->wCurrentUser==GetMeChairID()))
	{
		ActiveGameFrame();
		m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
		m_GameClientView.m_btPassCard.EnableWindow((m_bTurnCardCount>0)?TRUE:FALSE);
		m_GameClientView.m_btOutCard.EnableWindow((VerdictOutCard()==true)?TRUE:FALSE);
		m_GameClientView.m_btAutoOutCard.EnableWindow((m_bTurnCardCount>0)?TRUE:FALSE);
	}

	//播放声音
	if ((IsLookonMode()==true)||(pPassCard->wPassUser!=GetMeChairID()))	PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));

	//设置时间
	if (m_bTurnCardCount!=0)
	{
		BYTE bCardCount=m_bCardCount[pPassCard->wCurrentUser];
		SetGameTimer(pPassCard->wCurrentUser,IDI_OUT_CARD,(bCardCount<m_bTurnCardCount)?3:30);
	}
	else SetGameTimer(pPassCard->wCurrentUser,IDI_OUT_CARD,30);

	return true;
}

//游戏结束
bool CGameClientDlg::OnSubGameEnd(const void * pBuffer, WORD wDataSize)
{
	//效验数据
	ASSERT(wDataSize==sizeof(CMD_S_GameEnd));
	if (wDataSize!=sizeof(CMD_S_GameEnd)) return false;

	//消息处理
	CMD_S_GameEnd * pGameEnd=(CMD_S_GameEnd *)pBuffer;

	//删除定时器
	KillTimer(IDI_MOST_CARD);
	KillGameTimer(IDI_OUT_CARD);
	KillGameTimer(IDI_LAND_SCORE);

	//隐藏控件
	m_GameClientView.m_btOutCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btPassCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btOneScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btTwoScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btThreeScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btGiveUpScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btAutoOutCard.ShowWindow(SW_HIDE);

	//禁用控件
	m_GameClientView.m_btOutCard.EnableWindow(FALSE);
	m_GameClientView.m_btPassCard.EnableWindow(FALSE);

	//设置积分
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		const tagUserData * pUserData=GetUserData(i);
		m_GameClientView.m_ScoreView.SetGameScore(i,pUserData->szName,pGameEnd->lGameScore[i]);
	}
	m_GameClientView.m_LeaveCardControl[0].SetDisplayFlag(true);
	m_GameClientView.m_LeaveCardControl[1].SetDisplayFlag(true);

	m_GameClientView.m_ScoreView.SetGameTax(pGameEnd->lGameTax);
	m_GameClientView.m_ScoreView.ShowWindow(SW_SHOW);

	//设置扑克
	BYTE bCardPos=0;
	for (WORD i=0;i<GAME_PLAYER;i++)
	{
		WORD wViewChairID=SwitchViewChairID(i);
		if (wViewChairID==0)
		{
			m_GameClientView.m_LeaveCardControl[0].SetCardData(&pGameEnd->bCardData[bCardPos],pGameEnd->bCardCount[i]);
		}
		else if (wViewChairID==2) 
		{
			m_GameClientView.m_LeaveCardControl[1].SetCardData(&pGameEnd->bCardData[bCardPos],pGameEnd->bCardCount[i]);
		}
		bCardPos+=pGameEnd->bCardCount[i];
		if (pGameEnd->bCardCount[i]!=0)
		{
			m_GameClientView.SetPassFlag(wViewChairID,false);
			m_GameClientView.m_UserCardControl[wViewChairID].SetCardData(NULL,0);
		}
	}

	//显示扑克
	if (IsLookonMode()==true) m_GameClientView.m_HandCardControl.SetDisplayFlag(true);

	//播放声音
	WORD wMeChairID=GetMeChairID();
	LONG lMeScore=pGameEnd->lGameScore[GetMeChairID()];
	if (lMeScore>0L) PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_WIN"));
	else if (lMeScore<0L) PlayGameSound(AfxGetInstanceHandle(),TEXT("GAME_LOST"));
	else PlayGameSound(GetModuleHandle(NULL),TEXT("GAME_END"));

	//设置界面
	if (IsLookonMode()==false)
	{
		m_GameClientView.m_btStart.ShowWindow(SW_SHOW);
		SetGameTimer(GetMeChairID(),IDI_START_GAME,30);
	}
	m_GameClientView.ShowLandTitle(false);

	return true;
}

//出牌判断
bool CGameClientDlg::VerdictOutCard()
{
	//状态判断
	if (m_GameClientView.m_btOutCard.IsWindowVisible()==FALSE) return false;

	//获取扑克
	BYTE bCardData[21];
	BYTE bShootCount=(BYTE)m_GameClientView.m_HandCardControl.GetShootCard(bCardData,CountArray(bCardData));

	//出牌判断
	if (bShootCount>0L)
	{
		//分析类型
		BYTE bMagicCard;
		BYTE bCardType;
		if(m_GameLogic.IsHadRoguishCard(bCardData,bShootCount))
		{
			bCardType=m_GameLogic.GetMagicCardType(bCardData,bShootCount,&bMagicCard);
			//char *str= new char[100];
			//sprintf(str,"有赖子最终类型%d",bCardType);
			//MessageBox(str);
			//delete []str;
			//类型判断
			if (bCardType==CT_INVALID) return false;
			//跟牌判断
			if (m_bTurnCardCount==0) return true;
			//变赖子
			for(int i=0;i<bShootCount;i++)
			{
				if(bCardData[i]==0x43)
					bCardData[i]=bMagicCard;
			}
			m_GameLogic.SortCardList(bCardData,bShootCount);
			return m_GameLogic.CompareCard(bCardData,m_bTurnCardData,bShootCount,m_bTurnCardCount);
		}
		else
		{
			bCardType=m_GameLogic.GetCardType(bCardData,bShootCount);
			//char *str=new char[100];
			//sprintf(str,"最终类型%d",bCardType);
			//MessageBox(str);
			//delete []str;
			if (bCardType==CT_INVALID) return false;
			//跟牌判断
			if (m_bTurnCardCount==0) return true;
			return m_GameLogic.CompareCard(bCardData,m_bTurnCardData,bShootCount,m_bTurnCardCount);
		}
	}

	return false;
}

//自动出牌
bool CGameClientDlg::AutomatismOutCard()
{
	//先出牌者
	if (m_bTurnCardCount==0)
	{
		//控制界面
		KillGameTimer(IDI_OUT_CARD);
		m_GameClientView.m_btOutCard.ShowWindow(SW_HIDE);
		m_GameClientView.m_btOutCard.EnableWindow(FALSE);
		m_GameClientView.m_btPassCard.ShowWindow(SW_HIDE);
		m_GameClientView.m_btPassCard.EnableWindow(FALSE);
		m_GameClientView.m_btAutoOutCard.ShowWindow(SW_HIDE);
		m_GameClientView.m_btAutoOutCard.EnableWindow(FALSE);

		//发送数据
		CMD_C_OutCard OutCard;
		OutCard.bCardCount=1;
		OutCard.bCardData[0]=m_bHandCardData[m_bHandCardCount-1];
		SendData(SUB_C_OUT_CART,&OutCard,sizeof(OutCard)-sizeof(OutCard.bCardData)+OutCard.bCardCount*sizeof(BYTE));

		//预先处理
		PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));
		m_GameClientView.m_UserCardControl[1].SetCardData(OutCard.bCardData,OutCard.bCardCount);

		//预先删除
		BYTE bSourceCount=m_bHandCardCount;
		m_bHandCardCount-=OutCard.bCardCount;
		m_GameLogic.RemoveCard(OutCard.bCardData,OutCard.bCardCount,m_bHandCardData,bSourceCount);
		m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);
	}
	else OnPassCard(0,0);

	return true;
}

//定时器消息
void CGameClientDlg::OnTimer(UINT nIDEvent)
{
	if ((nIDEvent==IDI_MOST_CARD)&&(m_wMostUser!=INVALID_CHAIR))
	{
		//变量定义
		WORD wCurrentUser=m_wMostUser;
		m_wMostUser=INVALID_CHAIR;

		//删除定时器
		KillTimer(IDI_MOST_CARD);

		//设置界面
		m_GameClientView.SetPassFlag(INVALID_CHAIR,false);
		for (WORD i=0;i<GAME_PLAYER;i++) m_GameClientView.m_UserCardControl[i].SetCardData(NULL,0);

		//玩家设置
		if ((IsLookonMode()==false)&&(wCurrentUser==GetMeChairID()))
		{
			ActiveGameFrame();
			m_GameClientView.m_btOutCard.ShowWindow(SW_SHOW);
			m_GameClientView.m_btPassCard.ShowWindow(SW_SHOW);
			m_GameClientView.m_btPassCard.EnableWindow(FALSE);
			m_GameClientView.m_btAutoOutCard.ShowWindow(SW_SHOW);
			m_GameClientView.m_btAutoOutCard.EnableWindow(FALSE);
			m_GameClientView.m_btOutCard.EnableWindow((VerdictOutCard()==true)?TRUE:FALSE);
		}

		//设置时间
		SetGameTimer(wCurrentUser,IDI_OUT_CARD,30);

		return;
	}

	__super::OnTimer(nIDEvent);
}

//开始按钮
LRESULT CGameClientDlg::OnStart(WPARAM wParam, LPARAM lParam)
{
	//设置变量
	m_wBombTime=1;
	m_wTimeOutCount=0;
	m_bHandCardCount=0;
	m_bTurnCardCount=0;
	m_bTurnOutType=CT_INVALID;
	m_wMostUser=INVALID_CHAIR;
	memset(m_bHandCardData,0,sizeof(m_bHandCardData));
	memset(m_bTurnCardData,0,sizeof(m_bTurnCardData));

	//设置界面
	KillGameTimer(IDI_START_GAME);
	m_GameClientView.SetBaseScore(0L);
	m_GameClientView.ShowLandTitle(false);
	m_GameClientView.SetBombTime(m_wBombTime);
	m_GameClientView.SetCardCount(INVALID_CHAIR,0);
	m_GameClientView.SetLandUser(INVALID_CHAIR,0);
	m_GameClientView.SetLandScore(INVALID_CHAIR,0);
	m_GameClientView.SetPassFlag(INVALID_CHAIR,false);

	//隐藏控件
	m_GameClientView.m_btStart.ShowWindow(FALSE);
	m_GameClientView.m_ScoreView.ShowWindow(SW_HIDE);

	//设置扑克
	m_GameClientView.m_BackCardControl.SetCardData(NULL,0);
	m_GameClientView.m_HandCardControl.SetCardData(NULL,0);
	m_GameClientView.m_HandCardControl.SetPositively(false);
	m_GameClientView.m_LeaveCardControl[0].SetCardData(NULL,0);
	m_GameClientView.m_LeaveCardControl[1].SetCardData(NULL,0);
	for (WORD i=0;i<GAME_PLAYER;i++) m_GameClientView.m_UserCardControl[i].SetCardData(NULL,0);

	//发送消息
	SendUserReady(NULL,0);
	PlayGameSound(AfxGetInstanceHandle(),TEXT("Ready"));
	return 0;
}

//出牌消息
LRESULT CGameClientDlg::OnOutCard(WPARAM wParam, LPARAM lParam)
{
	//状态判断
	if ((m_GameClientView.m_btOutCard.IsWindowEnabled()==FALSE)||
		(m_GameClientView.m_btOutCard.IsWindowVisible()==FALSE)) return 0;

	memset(m_bHintCardData,0,sizeof(m_bHintCardData));
	//设置界面
	KillGameTimer(IDI_OUT_CARD);
	m_GameClientView.m_btOutCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btOutCard.EnableWindow(FALSE);
	m_GameClientView.m_btPassCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btPassCard.EnableWindow(FALSE);
	m_GameClientView.m_btAutoOutCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btAutoOutCard.EnableWindow(FALSE);

	//发送数据
	CMD_C_OutCard OutCard;
	OutCard.bCardCount=(BYTE)m_GameClientView.m_HandCardControl.GetShootCard(OutCard.bCardData,CountArray(OutCard.bCardData));
	SendData(SUB_C_OUT_CART,&OutCard,sizeof(OutCard)-sizeof(OutCard.bCardData)+OutCard.bCardCount*sizeof(BYTE));

	//预先显示
	PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));
	m_GameClientView.m_UserCardControl[1].SetCardData(OutCard.bCardData,OutCard.bCardCount);

	//预先删除
	BYTE bSourceCount=m_bHandCardCount;
	m_bHandCardCount-=OutCard.bCardCount;
	m_GameLogic.RemoveCard(OutCard.bCardData,OutCard.bCardCount,m_bHandCardData,bSourceCount);
	m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);

	return 0;
}

//放弃出牌
LRESULT CGameClientDlg::OnPassCard(WPARAM wParam, LPARAM lParam)
{
	//状态判断
	if (m_GameClientView.m_btPassCard.IsWindowEnabled()==FALSE) return 0;

	memset(m_bHintCardData,0,sizeof(m_bHintCardData));
	//设置界面
	KillGameTimer(IDI_OUT_CARD);
	m_GameClientView.m_btOutCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btOutCard.EnableWindow(FALSE);
	m_GameClientView.m_btPassCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btPassCard.EnableWindow(FALSE);
	m_GameClientView.m_btAutoOutCard.ShowWindow(SW_HIDE);
	m_GameClientView.m_btAutoOutCard.EnableWindow(FALSE);

	//发送数据
	SendData(SUB_C_PASS_CARD);

	//预先显示
	m_GameClientView.SetPassFlag(1,true);
	PlayGameSound(AfxGetInstanceHandle(),TEXT("OUT_CARD"));
	m_GameClientView.m_HandCardControl.SetCardData(m_bHandCardData,m_bHandCardCount);

	return 0;
}

//叫分消息
LRESULT CGameClientDlg::OnLandScore(WPARAM wParam, LPARAM lParam)
{
	//设置界面
	KillGameTimer(IDI_LAND_SCORE);
	m_GameClientView.m_btOneScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btTwoScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btThreeScore.ShowWindow(SW_HIDE);
	m_GameClientView.m_btGiveUpScore.ShowWindow(SW_HIDE);

	//发送数据
	CMD_C_LandScore LandScore;
	LandScore.bLandScore=(BYTE)wParam;
	SendData(SUB_C_LAND_SCORE,&LandScore,sizeof(LandScore));

	return 0;
}

//出牌提示
LRESULT CGameClientDlg::OnAutoOutCard(WPARAM wParam, LPARAM lParam)
{
	AutoOutCard(0);
	return 0;
}

//右键扑克
LRESULT CGameClientDlg::OnLeftHitCard(WPARAM wParam, LPARAM lParam)
{
	//设置控件
	bool bOutCard=VerdictOutCard();
	m_GameClientView.m_btOutCard.EnableWindow(bOutCard?TRUE:FALSE);

	return 0;
}

//左键扑克
LRESULT CGameClientDlg::OnRightHitCard(WPARAM wParam, LPARAM lParam)
{
	//用户出牌
	OnOutCard(0,0);

	return 0;
}

void CGameClientDlg::AutoOutCard(BYTE m_WhichOnsKindCard)
{
	if(m_bHandCardCount<m_bTurnCardCount)
		OnPassCard(0,0);
	if(m_bHintCardData[0]==0)
	{
		m_bHintCardCount=m_bTurnCardCount;
		CopyMemory(m_bHintCardData,m_bTurnCardData,m_bHintCardCount);
	}

	int i=0;
	BYTE m_bWhichKindSel=0;
	BYTE							m_bTempSCardCount=0;				//扑克数目
	BYTE							m_bTempSCardData[21];				//手上扑克
	BYTE							m_bTempDCardCount=0;				//扑克数目
	BYTE							m_bTempDCardData[21];				//手上扑克
	BYTE							m_bTempTCardCount=0;				//扑克数目
	BYTE							m_bTempTCardData[21];				//手上扑克
	BYTE							m_bTempFCardCount=0;				//扑克数目
	BYTE							m_bTempFCardData[21];				//手上扑克
	BYTE							m_bTempGetCardCount=0;				//扑克数目
	BYTE							m_bTempGetCardData[21];				//手上扑克
	BYTE							m_bTempGetCardIndex[21];			//手上扑克
	BYTE m_TempCard=0;
	//如果没有人出牌，不提示
	if(m_bHintCardCount==0)
		return;
	m_GameClientView.m_HandCardControl.ShootAllCard(false);	
	for(i=0;i<m_bHandCardCount;i++)
	{	
		BYTE m_GetCard=m_GameLogic.GetCardLogicValue(m_bHandCardData[i]);
		if(m_TempCard!=m_GetCard)
		{
			m_bTempSCardData[m_bTempSCardCount++]=m_bHandCardData[i];
			m_TempCard=m_GetCard;
		}
	}
	//获取对牌列表
	m_TempCard=0;
	for(i=0;i<m_bHandCardCount-1;i++)
	{	
		BYTE m_GetCard1=m_GameLogic.GetCardLogicValue(m_bHandCardData[i]);
		BYTE m_GetCard2=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+1]);
		if(m_TempCard!=m_GetCard1&&m_GetCard1==m_GetCard2&&m_GetCard1<16)
		{
			m_bTempDCardData[m_bTempDCardCount++]=m_bHandCardData[i];
			m_bTempDCardData[m_bTempDCardCount++]=m_bHandCardData[i+1];
			m_TempCard=m_GetCard1;
		}
	}
	//获取三张牌列表
	m_TempCard=0;
	for(i=0;i<m_bHandCardCount-2;i++)
	{	
		BYTE m_GetCard1=m_GameLogic.GetCardLogicValue(m_bHandCardData[i]);
		BYTE m_GetCard2=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+1]);
		BYTE m_GetCard3=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+2]);
		if(m_TempCard!=m_GetCard1&&m_GetCard1==m_GetCard2&&m_GetCard1==m_GetCard3)
		{
			m_bTempTCardData[m_bTempTCardCount++]=m_bHandCardData[i];
			m_bTempTCardData[m_bTempTCardCount++]=m_bHandCardData[i+1];
			m_bTempTCardData[m_bTempTCardCount++]=m_bHandCardData[i+2];
			m_TempCard=m_GetCard1;
		}
	}
	//获取四张牌列表
	m_TempCard=0;
	for(i=0;i<m_bHandCardCount-3;i++)
	{	
		BYTE m_GetCard1=m_GameLogic.GetCardLogicValue(m_bHandCardData[i]);
		BYTE m_GetCard2=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+1]);
		BYTE m_GetCard3=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+2]);
		BYTE m_GetCard4=m_GameLogic.GetCardLogicValue(m_bHandCardData[i+3]);
		if(m_TempCard!=m_GetCard1&&m_GetCard1==m_GetCard2&&m_GetCard1==m_GetCard3&&m_GetCard1==m_GetCard4)
		{
			m_bTempFCardData[m_bTempFCardCount++]=m_bHandCardData[i];
			m_bTempFCardData[m_bTempFCardCount++]=m_bHandCardData[i+1];
			m_bTempFCardData[m_bTempFCardCount++]=m_bHandCardData[i+2];
			m_bTempFCardData[m_bTempFCardCount++]=m_bHandCardData[i+3];
			m_TempCard=m_GetCard1;
		}
	}
	//根据所出牌类型判断
	i=0;
	BYTE bTurnOutType;
	if(m_GameLogic.IsHadRoguishCard(m_bHintCardData,m_bHintCardCount))
	{
		bTurnOutType=m_GameLogic.GetMagicCardType(m_bHintCardData,m_bHintCardCount);
	}
	else
	{
		bTurnOutType=m_GameLogic.GetCardType(m_bHintCardData,m_bHintCardCount);
	}
	switch(bTurnOutType)
	{
	case CT_SINGLE:
	case CT_ONE_LINE:
		if(m_WhichOnsKindCard==1)   //判断是不是具有唯一性
		{
			for(i=m_bTempSCardCount;i>0;i--)
			{
				if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempSCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
				{
					if((m_bWhichKindSel++)>1)
						i=0;
				}
			}
		}
		for(i=m_bTempSCardCount;i>0;i--)
		{
			if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempSCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
			{
				//判断是不是最合理的
				bool m_bIsHaveCard=false;
				for(int j=0;j<m_bTempDCardCount;j++)
				{
					for(int n=0;n<m_bHintCardCount;n++)
					{
						if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[j]))
							m_bIsHaveCard=true;
					}
				}
				//把最合理的情况保存起来
				if(m_bTempGetCardCount==0||!m_bIsHaveCard)
				{
					CopyMemory(m_bTempGetCardData,&m_bTempSCardData[i-m_bHintCardCount],m_bHintCardCount);
					m_bTempGetCardCount=m_bHintCardCount;
				}
				if(!m_bIsHaveCard)
					break;
			}
		}
		//是否做赖子处理
		if((m_bTempGetCardCount==0)&&(m_GameLogic.IsHadRoguishCard(m_bHandCardData,m_bHandCardCount)))
		{
			if(m_bHintCardCount==1)
				break;
			BYTE bSingleMagic[21];
			//赖子
			bSingleMagic[0]=0x43;	
			for(i=m_bTempSCardCount;i>0;i--)
			{
				if(i-m_bHintCardCount>=0)
				{
					CopyMemory(&bSingleMagic[1],&m_bTempSCardData[i-m_bHintCardCount+1],m_bHintCardCount-1);
					if(m_GameLogic.CompareCard(bSingleMagic,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
					{
						//判断是不是最合理的
						bool m_bIsHaveCard=false;
						for(int j=0;j<m_bTempDCardCount;j++)
						{
							for(int n=0;n<m_bHintCardCount;n++)
							{
								if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[j]))
									m_bIsHaveCard=true;
							}
						}
						//把最合理的情况保存起来
						if(m_bTempGetCardCount==0||!m_bIsHaveCard)
						{
							CopyMemory(m_bTempGetCardData,bSingleMagic,m_bHintCardCount);
							m_bTempGetCardCount=m_bHintCardCount;
						}
						if(!m_bIsHaveCard)
							break;
					}
				}
			}
		}
		break;
	case CT_DOUBLE:
	case CT_DOUBLE_LINE:
		if(m_WhichOnsKindCard==1)     //判断是不是具有唯一性
		{
			for(i=m_bTempDCardCount;i>0;i--)
			{
				if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempDCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
				{
					if((m_bWhichKindSel++)>1)
						i=0;
				}
			}
		}
		for(i=m_bTempDCardCount;i>0;i--)
		{
			if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempDCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
			{
				//判断是不是最合理的
				bool m_bIsHaveCard=false;
				for(int j=0;j<m_bTempTCardCount;j++)
				{
					for(int n=0;n<m_bHintCardCount;n++)
					{
						if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[j]))
						{
							m_bIsHaveCard=true;
							break;
						}
					}
				}

				if(m_bTempGetCardCount==0||!m_bIsHaveCard)
				{
					CopyMemory(m_bTempGetCardData,&m_bTempDCardData[i-m_bHintCardCount],m_bHintCardCount);
					m_bTempGetCardCount=m_bHintCardCount;
				}
				if(!m_bIsHaveCard)
					break;
			}
		}
		//是否做赖子处理
		if((m_bTempGetCardCount==0)&&(m_GameLogic.IsHadRoguishCard(m_bHandCardData,m_bHandCardCount)))
		{
			BYTE bDoubleMagic[21];
			//赖子
			bDoubleMagic[0]=0x43;	
			for(int k=m_bTempSCardCount-1;k>=0;k--)
			{
				if(k>=0)
				{
					bDoubleMagic[1]=m_bTempSCardData[k];
					//是否连对
					bool bAble=false;
					bool bCountAble=false;
					if(m_bHintCardCount>2)
					{
						BYTE bDMagicLine[21];
						CopyMemory(bDMagicLine,bDoubleMagic,2);
						//拷贝对子
						for(i=m_bTempDCardCount;i>0;i-=2)
						{
							if(i-m_bHintCardCount+2>=0)
							{
								bCountAble=true;
								CopyMemory(&bDMagicLine[2],&m_bTempDCardData[i-m_bHintCardCount+2],m_bHintCardCount-2);
								if(m_GameLogic.CompareCard(bDMagicLine,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
								{
									//判断是不是最合理的
									bool m_bIsHaveCard=false;
									for(int j=0;j<m_bTempTCardCount;j++)
									{
										for(int n=0;n<m_bHintCardCount-2;n++)
										{
											if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[j]))
											{
												m_bIsHaveCard=true;
												break;
											}
										}
									}
									if(!m_bIsHaveCard||m_bTempGetCardCount==0)
									{
										CopyMemory(&bDoubleMagic[2],&bDMagicLine[2],m_bHintCardCount-2);	
										bAble=true;
										break;
									}
								}
							}
						}
						//张数不够,退出
						if(!bCountAble)
						{
							break;
						}
						//无合适牌
						if(!bAble)
						{
							continue;
						}
					}
					if(m_GameLogic.CompareCard(bDoubleMagic,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
					{
						//判断是不是最合理的
						bool bIsHaveCard=false;
						for(int j=0;j<m_bTempDCardCount;j++)
						{
							for(int n=0;n<m_bHintCardCount;n++)
							{
								if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[k-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[j]))
									bIsHaveCard=true;
							}
						}
						//把最合理的情况保存起来
						if(m_bTempGetCardCount==0||!bIsHaveCard)
						{
							CopyMemory(m_bTempGetCardData,bDoubleMagic,m_bHintCardCount);
							m_bTempGetCardCount=m_bHintCardCount;
							break;
						}
						break;
					}
				}
			}
		}
		break;
	case CT_THREE:
	case CT_THREE_LINE:
		if(m_WhichOnsKindCard==1)           //判断是不是具有唯一性
		{
			for(i=m_bTempTCardCount;i>0;i--)
			{
				if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempTCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
				{
					if((m_bWhichKindSel++)>1)
						i=0;
				}
			}
		}
		for(i=m_bTempTCardCount;i>0;i--)
		{
			if(i-m_bHintCardCount>=0&&m_GameLogic.CompareCard(&m_bTempTCardData[i-m_bHintCardCount],m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
			{
				//判断是不是最合理的
				bool m_bIsHaveCard=false;
				for(int j=0;j<m_bTempFCardCount;j++)
				{
					for(int n=0;n<m_bHintCardCount;n++)
					{
						if(m_GameLogic.GetCardLogicValue(m_bTempTCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempFCardData[j]))
							m_bIsHaveCard=true;
					}
				}
				if(m_bTempGetCardCount==0||!m_bIsHaveCard)
				{
					CopyMemory(m_bTempGetCardData,&m_bTempTCardData[i-m_bHintCardCount],m_bHintCardCount);
					m_bTempGetCardCount=m_bHintCardCount;
				}
				if(!m_bIsHaveCard&&m_bTempGetCardCount!=0)
					break;
			}
		}
		//赖子提示
		if((m_bTempGetCardCount==0)&&(m_GameLogic.IsHadRoguishCard(m_bHandCardData,m_bHandCardCount)))
		{
			BYTE bThreeMagic[21];
			//赖子
			bThreeMagic[0]=0x43;	
			for(i=m_bTempDCardCount-2;i>0;i-=2)
			{
				if(i>=1)
				{
					CopyMemory(&bThreeMagic[1],&m_bTempDCardData[i],2);
					//char *str=new char[111];
					//sprintf(str,"i=%d,%d,%d,%d",i,bThreeMagic[0],bThreeMagic[1],bThreeMagic[2]);
					//MessageBox(str);
					//delete []str;
					//三连
					
					if(m_bHintCardCount>3)
					{
						bool bCountAble=false;
						bool bAble=false;
						BYTE bTMagicLine[21];
						CopyMemory(bTMagicLine,bThreeMagic,3);
						for(int k=m_bTempTCardCount;k>0;k-=3)
						{
							if(k-m_bHintCardCount+3>=0)
							{
								bCountAble=true;
								CopyMemory(&bTMagicLine[3],&m_bTempTCardData[k-m_bHintCardCount+3],m_bHintCardCount-3);
								//char *str=new char[111];
								//int k=m_GameLogic.GetMagicCardType(bTMagicLine,9);
								//sprintf(str,"类型%d...%d,%d,%d,%d,%d,%d,%d",k,bTMagicLine[0],bTMagicLine[1],bTMagicLine[2],bTMagicLine[3],bTMagicLine[4],bTMagicLine[5],bTMagicLine[6]);
								//MessageBox(str);
								//delete []str;
								if(m_GameLogic.CompareCard(bTMagicLine,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
								{
									//判断是否最佳
									bool m_bIsHaveCard=false;
									for(int j=0;j<m_bTempFCardCount;j++)
									{
										for(int n=0;n<m_bHintCardCount;n++)
										{
											if(m_GameLogic.GetCardLogicValue(m_bTempTCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempFCardData[j]))

												m_bIsHaveCard=true;
										}
									}
									if(m_bTempGetCardCount==0||!m_bIsHaveCard)
									{
										CopyMemory(&bThreeMagic[3],&bTMagicLine[3],m_bHintCardCount-3);
										bAble=true;
										break;
										//m_bTempGetCardCount=m_bHintCardCount;
									}
								}
							}
						}
						//张数不够,退出

						if(!bCountAble)
						{
							break;
						}
						//无合适牌
						if(!bAble)
						{
							continue;
						}
					}
					if(m_GameLogic.CompareCard(bThreeMagic,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
					{
						//判断是否最佳
						bool m_bIsHaveCard=false;
						for(int j=0;j<m_bTempTCardCount;j++)
						{
							for(int n=0;n<m_bHintCardCount;n++)
							{
								if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[j]))
								{
									m_bIsHaveCard=true;
									break;
								}
							}
						}

						if(m_bTempGetCardCount==0||!m_bIsHaveCard)
						{
							CopyMemory(m_bTempGetCardData,bThreeMagic,m_bHintCardCount);
							m_bTempGetCardCount=m_bHintCardCount;
						}
					}
				}
			}
		}
		break;
	case CT_THREE_LINE_TAKE_ONE:
	case CT_THREE_LINE_TAKE_DOUBLE:
		{
			//分析扑克
			tagAnalyseResult AnalyseResult;
			m_GameLogic.AnalysebCardData(m_bHintCardData,m_bHintCardCount,AnalyseResult);      
			if(m_WhichOnsKindCard==1)               //判断是不是具有唯一性
			{
				for(i=m_bTempTCardCount;i>0;i--)
				{
					if(i-AnalyseResult.bThreeCount*3>=0&&m_GameLogic.CompareCard(&m_bTempTCardData[i-AnalyseResult.bThreeCount*3],m_bHintCardData,AnalyseResult.bThreeCount*3,AnalyseResult.bThreeCount*3))
					{
						if((m_bWhichKindSel++)>1)
							i=0;
					}
				}
			}
			for(i=m_bTempTCardCount;i>0;i--)
			{
				if(i-AnalyseResult.bThreeCount*3>=0&&m_GameLogic.CompareCard(&m_bTempTCardData[i-AnalyseResult.bThreeCount*3],AnalyseResult.m_bTCardData,AnalyseResult.bThreeCount*3,AnalyseResult.bThreeCount*3))
				{
					//判断是不是最合理的
					bool m_bIsHaveCard=false;
					for(int j=0;j<m_bTempFCardCount;j++)
					{
						for(int n=0;n<AnalyseResult.bThreeCount*3;n++)
						{
							if(m_GameLogic.GetCardLogicValue(m_bTempTCardData[i-AnalyseResult.bThreeCount*3+n])==m_GameLogic.GetCardLogicValue(m_bTempFCardData[j]))
								m_bIsHaveCard=true;
						}
					}
					if(m_bTempGetCardCount==0||!m_bIsHaveCard)
					{
						CopyMemory(m_bTempGetCardData,&m_bTempTCardData[i-AnalyseResult.bThreeCount*3],AnalyseResult.bThreeCount*3);
						m_bTempGetCardCount=AnalyseResult.bThreeCount*3;
					}
					if(!m_bIsHaveCard&&m_bTempGetCardCount!=0)
						i=0;
				}
			}
			if(m_bTempGetCardCount>0)
			{
				bool m_bIsHaveSame;
				for(int m=0;m<AnalyseResult.bDoubleCount;m++)
				{
					for(int j=0;j<m_bTempDCardCount/2;j++)
					{
						//判断是不是最合理的
						m_bIsHaveSame=false;
						int n;
						for(n=0;n<m_bTempGetCardCount;n++)
						{
							if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[m_bTempDCardCount-j*2-1])==m_GameLogic.GetCardLogicValue(m_bTempGetCardData[n]))
							{
								m_bIsHaveSame=true;
							}
						}
						if(!m_bIsHaveSame)
						{
							bool m_bIsHaveCard=false;
							for(int s=0;s<m_bTempTCardCount;s++)
							{
								for(n=0;n<m_bTempGetCardCount;n++)
								{
									if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[m_bTempDCardCount-j*2-1])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[s]))
										m_bIsHaveCard=true;
								}
							}
							if(m_bTempGetCardCount==AnalyseResult.bThreeCount*3||!m_bIsHaveCard)
							{
								m_bTempGetCardData[AnalyseResult.bThreeCount*3+m*2]=m_bTempDCardData[m_bTempDCardCount-j*2-1];
								m_bTempGetCardData[AnalyseResult.bThreeCount*3+m*2+1]=m_bTempDCardData[m_bTempDCardCount-j*2-2];
								m_bTempGetCardCount=AnalyseResult.bThreeCount*3+(m+1)*2;
							}
							if(!m_bIsHaveCard)
							{
								n=m_bTempGetCardCount;
								j=m_bTempDCardCount/2;
							}
						}
					}
				}
				for(int m=0;m<AnalyseResult.bSignedCount;m++)
				{
					for(int j=0;j<m_bTempSCardCount;j++)
					{
						//判断是不是最合理的
						m_bIsHaveSame=false;
						int n;
						for(n=0;n<m_bTempGetCardCount;n++)
						{
							if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[m_bTempSCardCount-j-1])==m_GameLogic.GetCardLogicValue(m_bTempGetCardData[n]))
							{
								m_bIsHaveSame=true;
							}
						}
						if(!m_bIsHaveSame)
						{
							bool m_bIsHaveCard=false;
							for(int s=0;s<m_bTempDCardCount;s++)
							{
								for(n=0;n<m_bTempGetCardCount;n++)
								{
									if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[m_bTempSCardCount-j-1])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[s]))
										m_bIsHaveCard=true;
								}
							}
							if(m_bTempGetCardCount==AnalyseResult.bThreeCount*3||!m_bIsHaveCard)
							{
								m_bTempGetCardData[AnalyseResult.bThreeCount*3+m]=m_bTempSCardData[m_bTempSCardCount-j-1];
								m_bTempGetCardCount=AnalyseResult.bThreeCount*3+m+1;
							}
							if(!m_bIsHaveCard)
							{
								n=m_bTempGetCardCount;
								j=m_bTempSCardCount;
							}
						}
					}
				}
			}
		}
		//如果有赖子
		if((m_bTempGetCardCount==0)&&(m_GameLogic.IsHadRoguishCard(m_bHandCardData,m_bHandCardCount)))
		{
			BYTE bMagicTake[21];
			memset(bMagicTake,0,21);
			bMagicTake[0]=0x43;
			if(bTurnOutType==CT_THREE_LINE_TAKE_ONE)
			{
				//取单张
				for(i=m_bTempSCardCount-1;i>=0;i--)
				{
					//判断是不是最合理的
					bool m_bIsHaveCard=false;
					for(int j=0;j<m_bTempDCardCount;j++)
					{
						if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[i])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[j]))
							m_bIsHaveCard=true;
					}
					//把最合理的情况保存起来
					if(bMagicTake[1]==0||!m_bIsHaveCard)
					{
						bMagicTake[1]=m_bTempSCardData[i];
						break;
					}
				}
				//取对牌
				for(int j=m_bTempDCardCount-1;j>=0;j--)
				{
					CopyMemory(&bMagicTake[2],&m_bTempDCardData[j],2);
					//char *str=new char[111];
					//sprintf(str,"%d,%d,%d,%d",bMagicTake[0],bMagicTake[1],bMagicTake[2],bMagicTake[3],bMagicTake[4]);
					//MessageBox(str);
					//delete []str;
					if(m_GameLogic.CompareCard(bMagicTake,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
					{
						//判断是不是最合理的
						bool m_bIsHaveCard=false;
						for(int j=0;j<m_bTempTCardCount;j++)
						{
							for(int n=0;n<m_bHintCardCount;n++)
							{
								if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[j]))
								{
									m_bIsHaveCard=true;
									break;
								}
							}
						}
						if(m_bTempGetCardCount==0||!m_bIsHaveCard)
						{
							CopyMemory(m_bTempGetCardData,bMagicTake,m_bHintCardCount);
							m_bTempGetCardCount=m_bHintCardCount;
						}
						if(!m_bIsHaveCard)
							break;
					}
				}
			}

			if(bTurnOutType==CT_THREE_LINE_TAKE_DOUBLE)
			{
				//取对牌
				for(int j=m_bTempDCardCount-1;j>=0;j--)
				{
					CopyMemory(&bMagicTake[1],&m_bTempDCardData[j],4);
					//char *str=new char[111];
					//sprintf(str,"%d,%d,%d,%d",bMagicTake[0],bMagicTake[1],bMagicTake[2],bMagicTake[3],bMagicTake[4]);
					//MessageBox(str);
					/*delete []str;*/
					if(m_GameLogic.CompareCard(bMagicTake,m_bHintCardData,m_bHintCardCount,m_bHintCardCount))
					{
						//判断是不是最合理的
						bool m_bIsHaveCard=false;
						for(int j=0;j<m_bTempTCardCount;j++)
						{
							for(int n=0;n<m_bHintCardCount;n++)
							{
								if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[i-m_bHintCardCount+n])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[j]))
								{
									m_bIsHaveCard=true;
									break;
								}
							}
						}
						if(m_bTempGetCardCount==0||!m_bIsHaveCard)
						{
							CopyMemory(m_bTempGetCardData,bMagicTake,m_bHintCardCount);
							m_bTempGetCardCount=m_bHintCardCount;
						}
						if(!m_bIsHaveCard)
							break;
					}
				}
			}
		}
		break;
	case CT_FOUR_LINE_TAKE_ONE:
	case CT_FOUR_LINE_TAKE_DOUBLE:
		{
			//分析扑克
			tagAnalyseResult AnalyseResult;
			m_GameLogic.AnalysebCardData(m_bHintCardData,m_bHintCardCount,AnalyseResult);
			if(m_WhichOnsKindCard==1)       //判断是不是具有唯一性
			{
				for(i=m_bTempFCardCount;i>0;i--)
				{
					if(i-AnalyseResult.bFourCount*4>=0&&m_GameLogic.CompareCard(&m_bTempFCardData[i-AnalyseResult.bFourCount*4],m_bHintCardData,AnalyseResult.bFourCount*4,AnalyseResult.bFourCount*4))
					{
						if((m_bWhichKindSel++)>1)
							i=0;
					}
				}
			}
			for(i=m_bTempFCardCount;i>0;i--)
			{
				if(i-AnalyseResult.bFourCount*4>=0&&m_GameLogic.CompareCard(&m_bTempFCardData[i-AnalyseResult.bFourCount*4],m_bHintCardData,AnalyseResult.bFourCount*4,AnalyseResult.bFourCount*4))
				{
					CopyMemory(m_bTempGetCardData,&m_bTempFCardData[i-AnalyseResult.bFourCount*4],AnalyseResult.bFourCount*4);
					m_bTempGetCardCount=AnalyseResult.bFourCount*4;
					i=0;
				}
			}
			if(m_bTempGetCardCount>0)
			{
				bool m_bIsHaveSame;
				for(int m=0;m<AnalyseResult.bDoubleCount;m++)
				{
					for(int j=0;j<m_bTempDCardCount/2;j++)
					{
						//判断是不是最合理的
						m_bIsHaveSame=false;
						int n;
						for(n=0;n<m_bTempGetCardCount;n++)
						{
							if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[m_bTempDCardCount-j*2-1])==m_GameLogic.GetCardLogicValue(m_bTempGetCardData[n]))
							{
								m_bIsHaveSame=true;
							}
						}
						if(!m_bIsHaveSame)
						{
							bool m_bIsHaveCard=false;
							for(int s=0;s<m_bTempTCardCount;s++)
							{
								for(n=0;n<m_bTempGetCardCount;n++)
								{
									if(m_GameLogic.GetCardLogicValue(m_bTempDCardData[m_bTempDCardCount-j*2-1])==m_GameLogic.GetCardLogicValue(m_bTempTCardData[s]))
										m_bIsHaveCard=true;
								}
							}
							if(m_bTempGetCardCount==AnalyseResult.bFourCount*4||!m_bIsHaveCard)
							{
								m_bTempGetCardData[AnalyseResult.bFourCount*4+m*2]=m_bTempDCardData[m_bTempDCardCount-j*2-1];
								m_bTempGetCardData[AnalyseResult.bFourCount*4+m*2+1]=m_bTempDCardData[m_bTempDCardCount-j*2-2];
								m_bTempGetCardCount=AnalyseResult.bFourCount*4+(m+1)*2;
							}
							if(!m_bIsHaveCard)
							{
								n=m_bTempGetCardCount;
								j=m_bTempDCardCount/2;
							}
						}
					}
				}
				for(int m=0;m<AnalyseResult.bSignedCount;m++)
				{
					for(int j=0;j<m_bTempSCardCount;j++)
					{
						//判断是不是最合理的
						m_bIsHaveSame=false;
						int n;
						for(n=0;n<m_bTempGetCardCount;n++)
						{
							if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[m_bTempSCardCount-j-1])==m_GameLogic.GetCardLogicValue(m_bTempGetCardData[n]))
							{
								m_bIsHaveSame=true;
							}
						}
						if(!m_bIsHaveSame)
						{
							bool m_bIsHaveCard=false;
							for(int s=0;s<m_bTempDCardCount;s++)
							{
								for(n=0;n<m_bTempGetCardCount;n++)
								{
									if(m_GameLogic.GetCardLogicValue(m_bTempSCardData[m_bTempSCardCount-j-1])==m_GameLogic.GetCardLogicValue(m_bTempDCardData[j]))
										m_bIsHaveCard=true;
								}
							}
							if(m_bTempGetCardCount==AnalyseResult.bFourCount*4||!m_bIsHaveCard)
							{
								m_bTempGetCardData[AnalyseResult.bFourCount*4+m]=m_bTempSCardData[m_bTempSCardCount-j-1];
								m_bTempGetCardCount=AnalyseResult.bFourCount*4+m+1;
							}
							if(!m_bIsHaveCard)
							{
								n=m_bTempGetCardCount;
								j=m_bTempSCardCount;
							}
						}
					}
				}
			}
		}
		break;
	}
	bool bBomb=false;		//是否出了炸
	bool bMissile=false;	//是否火煎
	if(m_bTempGetCardCount==0)
	{
		m_bWhichKindSel=0;
		//赖子炸
		if((m_bTempGetCardCount==0)&&(m_GameLogic.IsHadRoguishCard(m_bHandCardData,m_bHandCardCount)))
		{
			if(m_bTempTCardCount>3)
			{
				BYTE bMagicBomb[21];
				bMagicBomb[0]=0x43;
				for(i=m_bTempTCardCount-3;i>=0;i--)
				{
					CopyMemory(&bMagicBomb[1],&m_bTempTCardData[i],3);
					if(m_GameLogic.CompareCard(bMagicBomb,m_bHintCardData,4,m_bHintCardCount))
					{
						if((m_bWhichKindSel++)==0)
						{
							CopyMemory(m_bTempGetCardData,bMagicBomb,4);
							bBomb=true;
							m_bTempGetCardCount=4;
							break;
						}
					}
				}
			}
		}
		//判断炸弹的可能性
		if(m_bTempGetCardCount==0)
		{
			if(m_bTempFCardCount>3)
			{
				for(i=m_bTempFCardCount-4;i>=0;i--)
				{
					if(m_GameLogic.CompareCard(&m_bTempFCardData[i],m_bHintCardData,4,m_bHintCardCount))
					{
						if((m_bWhichKindSel++)==0)
						{
							CopyMemory(m_bTempGetCardData,&m_bTempFCardData[i],4);
							bBomb=true;
							m_bTempGetCardCount=4;
						}
					}
				}
			}
		}
		//火箭
		if(m_bTempGetCardCount==0)
		{
			if(m_bHandCardCount>1)
			{
				for(int i=0; i<2;i++)
				{
					if(m_GameLogic.GetCardLogicValue(m_bHandCardData[i])>15&&m_GameLogic.GetCardLogicValue(m_bHandCardData[i+1])>15)
					{
						CopyMemory(m_bTempGetCardData,&m_bHandCardData[i],2);
						bMissile=true;
						m_bTempGetCardCount=2;
						if(m_WhichOnsKindCard==1)
							m_bWhichKindSel=1;
					}
				}
			}
		}
	}
	BYTE m_GetIndex=0;
	if(m_bTempGetCardCount==0)
	{
		if(m_WhichOnsKindCard!=1)
		{
			if(m_bHintCardData[0]!=m_bTurnCardData[0])
			{
				memset(m_bHintCardData,0,m_bHintCardCount);
				AutoOutCard(0);
			}
			else
				OnPassCard(0,0);	
		}
	}
	else
	{
		for(int j=0;j<m_bTempGetCardCount;j++)
		{
			for(i=0;i<m_bHandCardCount;i++)
			{
				if(m_bHandCardData[i]==m_bTempGetCardData[j])
				{
					m_bTempGetCardIndex[m_GetIndex++]=i;
				}
			}
		}

	}
	if(m_GameLogic.CompareCard(m_bTempGetCardData,m_bHintCardData,m_bTempGetCardCount,m_bHintCardCount))
	{
		if(m_WhichOnsKindCard==1&&m_bWhichKindSel==1||m_WhichOnsKindCard!=1)
		{
			//拷贝提示出牌
			if(bBomb)
				m_bHintCardCount=4;
			if(bMissile)
				m_bHintCardCount=2;
			CopyMemory(m_bHintCardData,m_bTempGetCardData,m_bHintCardCount);
			m_GameClientView.m_HandCardControl.SetShootCard(m_bTempGetCardIndex,m_GetIndex);
			m_GameClientView.m_btOutCard.EnableWindow(TRUE);
		}
	}
	else
	{
		if(m_WhichOnsKindCard!=1)
		{	
			if(m_bHintCardData[0]!=m_bTurnCardData[0])
			{
				memset(m_bHintCardData,0,m_bHintCardCount);
				AutoOutCard(0);
			}
			else
				OnPassCard(0,0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////

