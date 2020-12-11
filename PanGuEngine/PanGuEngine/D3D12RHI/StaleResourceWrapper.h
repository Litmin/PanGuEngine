#pragma once

// 封装应用程序释放的资源，可以把不同类型的资源放到同一个队列中
class StaleResourceWrapper final
{
public:
	template <typename ResourceType >
	static StaleResourceWrapper Create(ResourceType&& resource)
	{
		// 类模板
		class SpecificStaleResource final : public StaleResourceBase
		{
		public:
			SpecificStaleResource(ResourceType resource) :
				m_SpecificResource(std::move(resource))
			{}

			SpecificStaleResource(const SpecificStaleResource&)				= delete;
			SpecificStaleResource(SpecificStaleResource&&)					= delete;
			SpecificStaleResource& operator= (const specificStaleResource&) = delete;
			SpecificStaleResource& operator= (SpecificStaleResource&&)		= delete;

			// delete this!!!!!!
			virtual void Release() override	final
			{
				delete this;
			}

		private:
			ResourceType m_SpecificResource;
		};

		return StaleResourceWrapper(new SpecificStaleResource(resource));
	}

	StaleResourceWrapper(StaleResourceWrapper&& rhs) noexcept :
		m_StaleResource(rhs.m_StaleResource)
	{
		rhs.m_StaleResource = nullptr;
	}

	StaleResourceWrapper(const StaleResourceWrapper& rhs) = delete;
	StaleResourceWrapper& operator= (const StaleResourceWrapper&)  = delete;
	StaleResourceWrapper& operator= (const StaleResourceWrapper&&) = delete;

	~StaleResourceWrapper()
	{
		if (m_StaleResource != nullptr)
			m_StaleResource->Release();
	}

	void GiveUpOwnership()
	{
		m_StaleResource = nullptr;
	}

private:
	// 释放的资源的基类，Wrapper对象持有一个基类指针
	class StaleResourceBase
	{
	public:
		virtual void Release() = 0;
	};

	// 私有的构造函数，只能通过Create创建Wrapper对象
	StaleResourceWrapper(StaleResourceBase* staleResource) :
		m_StaleResource(staleResource)
	{
	
	}

	StaleResourceBase* m_StaleResource;
};

