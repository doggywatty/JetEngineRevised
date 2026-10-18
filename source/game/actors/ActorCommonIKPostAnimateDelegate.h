// source/game/actors/ActorCommonIKPostAnimateDelegate.h
#ifndef SOURCE_GAME_ACTORS_ACTORCOMMONIKPOSTANIMATEDELEGATE_H
#define SOURCE_GAME_ACTORS_ACTORCOMMONIKPOSTANIMATEDELEGATE_H

class ActorCommonIKPostAnimateDelegate : public jet::scene::Model::PostAnimateDelegate {  // sizeof 0x70
public:
    ActorCommonIKPostAnimateDelegate();
    ~ActorCommonIKPostAnimateDelegate();
    virtual void Init();
    void SetBlendAnim(jet::anim::Animation*, int);
    void SetBlendAnim(const jet::String&);
    void SetBlendAnimStrength(float, float);
    void StopBlendAnim();
    jet::anim::Animation *GetDirBlendAnim();
    void RenderDebug();
    bool IsBlendAnimEnabled();
    void SetMergeAnim(jet::anim::Animation*, int);
    void SetMergeAnim(const jet::String&);
    bool IsPlayingMergeAnim();
    void SetAvoidNodes(bool);
    virtual void Process();
    virtual void Undo();
protected:
    void ProcessBlendAnim();
    void UndoBlendAnim();
    void InitBlendAnim();
    void ProcessMergeAnim();
    void UndoMergeAnim(bool);
    void InitMergeAnim();
    void AddNodeToMerge(jet::scene::Node*, jet::scene::Model*);
    void AddAvoidNode(jet::scene::Node*);
    void InitAvoidNodes();
    void ProcessAvoidNodes();
public:
    static const int k_mergeAnimBlendTimeStart;
    static const int k_mergeAnimBlendTimeEnd;
protected:
    std::vector<ActorCommonIKPostAnimateDelegate::IKAnimNode,std::allocator<ActorCommonIKPostAnimateDelegate::IKAnimNode> > m_blendAnimNodes;  // 0x8
    jet::anim::Animation *m_blendBaseAnim;  // 0x1C
    jet::anim::Animation *m_blendAnim;  // 0x20
    int m_blendBaseAnimTime;  // 0x24
    unsigned m_blendStartTime;  // 0x28
    float m_blendAnimStrength;  // 0x2C
    bool m_applyBlendAnim;  // 0x30
    bool m_canUndoBlendAnim;  // 0x31
    bool m_blendAnimAutoControl;  // 0x32
    std::vector<ActorCommonIKPostAnimateDelegate::IKAnimNode,std::allocator<ActorCommonIKPostAnimateDelegate::IKAnimNode> > m_mergeAnimNodes;  // 0x34
    jet::anim::Animation *m_mergeBaseAnim;  // 0x48
    jet::anim::Animation *m_mergeAnim;  // 0x4C
    int m_mergeBaseAnimTime;  // 0x50
    bool m_mergeApply;  // 0x54
    bool m_mergeCanUndo;  // 0x55
    std::vector<ActorCommonIKPostAnimateDelegate::IKBackupNode,std::allocator<ActorCommonIKPostAnimateDelegate::IKBackupNode> > m_avoidNodes;  // 0x58
    bool m_avoidNodesEnabled;  // 0x6C
};

#endif
