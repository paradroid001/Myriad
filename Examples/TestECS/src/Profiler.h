#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <memory>

// 1. Data Node for Hierarchy
struct ProfileResult
{
  std::string name;
  long long start;
  long long end;
  std::vector<ProfileResult> children;
};

// 2. Global Profile Manager
class ProfilerManager
{
public:
  static ProfilerManager &Get()
  {
    static ProfilerManager instance;
    return instance;
  }

  void StartFrame()
  {
    currentFrameData.children.clear();
    currentFrameData.name = "Root";
    currentNode = &currentFrameData;
  }

  void EndFrame()
  {
    // Output or pass this tree structure directly to an ImGui renderer
    PrintNode(currentFrameData, 0);
  }

  void PushResult(const std::string &name, long long start, long long end)
  {
    // If we want a deep hierarchical tree, we push to the active scope node.
    // For simplicity in this flat/one-level deep example, we append to the current node.
    ProfileResult result{name, start, end, {}};
    currentNode->children.push_back(result);
  }

private:
  ProfileResult currentFrameData;
  ProfileResult *currentNode = nullptr;

  void PrintNode(const ProfileResult &node, int depth)
  {
    for (int i = 0; i < depth; ++i)
      std::cout << "  ";
    if (node.name != "Root")
    {
      long long duration = node.end - node.start;
      std::cout << node.name << ": " << duration << "us\n";
    }
    for (const auto &child : node.children)
    {
      PrintNode(child, depth + 1);
    }
  }
};

// 3. RAII Scope Timer
class ProfileScope
{
public:
  ProfileScope(const std::string &name) : m_Name(name)
  {
    m_StartTimepoint = std::chrono::high_resolution_clock::now();
  }

  ~ProfileScope()
  {
    auto endTimepoint = std::chrono::high_resolution_clock::now();
    long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
    long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

    ProfilerManager::Get().PushResult(m_Name, start, end);
  }

private:
  std::string m_Name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
};
