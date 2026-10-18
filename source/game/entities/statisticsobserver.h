// source/game/entities/statisticsobserver.h
#ifndef SOURCE_GAME_ENTITIES_STATISTICSOBSERVER_H
#define SOURCE_GAME_ENTITIES_STATISTICSOBSERVER_H

class StatisticsObserver {  // sizeof 0xC
public:
    StatisticsObserver();
    virtual ~StatisticsObserver();
    virtual void Update(const jet::String&, const GameplayStatisticsValue&) = 0;
    safe_enum<EStatisticTypeDef,enum EStatisticTypeDef::type> GetObservedStatType();
    void SetObservedStatType(safe_enum<EStatisticTypeDef,enum EStatisticTypeDef::type>);
    void SetIsRegistered(bool);
    bool IsRegistered();
private:
    safe_enum<EStatisticTypeDef,enum EStatisticTypeDef::type> m_observedStatType;  // 0x4
    bool m_isRegistered;  // 0x8
};

#endif
