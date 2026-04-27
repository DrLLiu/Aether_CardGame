// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelNode.h"


ULevelNode::ULevelNode()
{
    //生成GUID
    NodeGUID = FGuid::NewGuid();

    //基于GUID生成临时唯一ID
    NodeID = FName(*FString::Printf(TEXT("Node_%s"), *NodeGUID.ToString()));

    // 默认状态
    NodeType = ELevelNodeType::COMBAT;//默认为战斗，防止出现奇奇怪怪的bug
    Column = 0;
    FloorLevel = 0;
    bIsUnlocked = false;
    bIsCompleted = false;
}

void ULevelNode::AddParentNode(ULevelNode* ParentNode)
{
    if (ParentNode && !ParentNodes.Contains(ParentNode))
    {
        ParentNodes.Add(ParentNode);

        // 双向连接：确保父节点也知道这个子节点
        if (!ParentNode->ChildNodes.Contains(this))
        {
            ParentNode->ChildNodes.Add(this);
        }
    }
}

void ULevelNode::AddChildNode(ULevelNode* ChildNode)
{
    if (ChildNode && !ChildNodes.Contains(ChildNode))
    {
        ChildNodes.Add(ChildNode);

        // 双向连接：确保子节点也知道这个父节点
        if (!ChildNode->ParentNodes.Contains(this))
        {
            ChildNode->ParentNodes.Add(this);
        }
    }
}

void ULevelNode::SetUnlocked(bool bUnlocked)
{
    bIsUnlocked = bUnlocked;

    // 解锁逻辑：如果解锁，检查是否可以连锁解锁子节点
    if (bUnlocked)
    {
        // 这里可以添加解锁时的额外逻辑
        // 例如：播放解锁音效、触发事件等，暂时不考虑最后看心情再做
    }
}

void ULevelNode::SetCompleted(bool bCompleted)
{
    bIsCompleted = bCompleted;

    // 完成逻辑：解锁所有子节点
    if (bCompleted)
    {
        for (ULevelNode* Child : ChildNodes)
        {
            if (Child && !Child->bIsUnlocked)
            {
                Child->SetUnlocked(true);
            }
        }
    }
}

bool ULevelNode::IsReachable() const
{
    // 起始节点总是可达
    if (FloorLevel == 0)
        return true;

    // 检查是否有已完成的父节点
    for (const ULevelNode* Parent : ParentNodes)
    {
        if (Parent && Parent->bIsCompleted)
        {
            return true;
        }
    }

    return false;
}

void ULevelNode::AddBuff(const FNodeBuff& NewBuff)
{
    // 检查是否已存在相同ID的Buff
    int32 ExistingIndex = AvailableBuffs.IndexOfByPredicate(
        [&NewBuff](const FNodeBuff& Buff) { return Buff.BuffID == NewBuff.BuffID; });

    if (ExistingIndex != INDEX_NONE)
    {
        // 已存在，更新
        AvailableBuffs[ExistingIndex] = NewBuff;
    }
    else
    {
        // 不存在，添加
        AvailableBuffs.Add(NewBuff);
    }
}

void ULevelNode::ClearBuffs()
{
    AvailableBuffs.Empty();
}