#include "PlayerScopeManager.h"
#include "IoCHelper.h"

namespace
{
const std::string DefaultScope = "default";
}

PlayerScopeManager::PlayerScopeManager(IObjectRepositoryPtr objRepository) : _playerId(DefaultScope), _objRepository(objRepository)
{
}

void PlayerScopeManager::createPlayerScope(const std::string& playerId)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_playerScopes.find(playerId) != _playerScopes.end()) {
        throw std::runtime_error("Player scope already exists: " + playerId);
    }

    const std::string scopeName = playerId + "_scope";
    auto createScopeCmd = IoC::Resolve<ICommandPtr>("Scopes.New", {scopeName});
    createScopeCmd->exec();

    auto setScopeCmd = IoC::Resolve<ICommandPtr>("Scopes.Current",{scopeName});
    setScopeCmd->exec();

    registerPlayerDependencies(playerId);

    setDefaultScope();

    _playerScopes[playerId] = scopeName;
}

void PlayerScopeManager::setCurrentPlayerScope(const std::string& playerId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_playerScopes.find(playerId) == _playerScopes.end()) {
        throw std::runtime_error("Player scope not found: " + playerId);
    }
    
    auto setScopeCmd = IoC::Resolve<ICommandPtr>("Scopes.Current",{playerId + "_scope"});
    setScopeCmd->exec();
    
    _playerId = playerId;
}

void PlayerScopeManager::setDefaultScope()
{
    auto setScopeCmd = IoC::Resolve<ICommandPtr>("Scopes.Current",{DefaultScope});
    setScopeCmd->exec();

    _playerId = DefaultScope;
}

std::string PlayerScopeManager::getCurrentPlayer()
{
    return _playerId;
}

void PlayerScopeManager::destroyPlayerScope(const std::string& playerId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _playerScopes.find(playerId);
    if (it == _playerScopes.end()) {
        return;
    }
    
    auto clearCmd = IoC::Resolve<ICommandPtr>("Scopes.Clear",{playerId + "_scope"});
    clearCmd->exec();
    
    _playerScopes.erase(it);
    
    if (_playerId == playerId)
    {
        setDefaultScope();
    }
}

void PlayerScopeManager::registerPlayerDependencies(const std::string& playerId)
{
    registerFactoryHelper("GameObjects.Get",
        [this, playerId](const std::vector<AnyValue>& args) -> UObjectPtr {
            std::string objectId = std::any_cast<std::string>(args[0]);

            auto obj = getObject(playerId, objectId);
            if (!obj) {
                throw std::runtime_error("Unrestricted access by " + playerId + " to " + objectId);
            }

            return getObject(playerId, objectId);
        }
    )->exec();

    registerFactoryHelper("Command.CheckAvailable",
        [playerId](const std::vector<AnyValue>& args) -> bool {
            std::string commandId = std::any_cast<std::string>(args[0]);
            
            std::vector<std::string> commands = {
                "StartMove", "StopMove", "Fire"
            };
            
            auto cmd = std::find(commands.begin(), commands.end(), commandId);            
            return cmd != commands.end();
        }
    )->exec();
}

UObjectPtr PlayerScopeManager::getObject(const std::string& playerId, const std::string& objectId)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _objRepository->getObject(playerId, objectId);
}
