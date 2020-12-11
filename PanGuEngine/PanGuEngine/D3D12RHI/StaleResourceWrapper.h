#pragma once

// ��װӦ�ó����ͷŵ���Դ�����԰Ѳ�ͬ���͵���Դ�ŵ�ͬһ��������
class StaleResourceWrapper final
{
public:
	template <typename ResourceType >
	static StaleResourceWrapper Create(ResourceType&& resource)
	{
		// ��ģ��
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
	// �ͷŵ���Դ�Ļ��࣬Wrapper�������һ������ָ��
	class StaleResourceBase
	{
	public:
		virtual void Release() = 0;
	};

	// ˽�еĹ��캯����ֻ��ͨ��Create����Wrapper����
	StaleResourceWrapper(StaleResourceBase* staleResource) :
		m_StaleResource(staleResource)
	{
	
	}

	StaleResourceBase* m_StaleResource;
};

