namespace util
{

	/// <summary>
	/// Created by dbasin on 7/3/16.
	/// </summary>
	template<typename L, typename R>
	class Pair
	{

	private:
		const L left;
		const R right;

	public:
		Pair(L left, R right) : left(left), right(right)
		{
		}

		virtual L getLeft()
		{
			return left;
		}
		virtual R getRight()
		{
			return right;
		}

		int hashCode() override
		{
			return left.hashCode() ^ right.hashCode();
		}
		bool equals(void *o) override
		{
			if (!(dynamic_cast<Pair*>(o) != nullptr))
			{
				return false;
			}
			Pair *pairo = static_cast<Pair*>(o);
			return this->left.equals(pairo->getLeft()) && this->right.equals(pairo->getRight());
		}

	};
}
