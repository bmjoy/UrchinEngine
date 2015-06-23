#include <boost/date_time.hpp>
#include <chrono>

#include "PhysicsWorld.h"

namespace urchin
{

	PhysicsWorld::PhysicsWorld() :
			physicsSimulationThread(nullptr),
			physicsSimulationStopper(false),
			timeStep(0),
			maxSubStep(1),
			bodyManager(new BodyManager()),
			collisionWorld(new CollisionWorld(bodyManager))
	{
		#ifdef _DEBUG
			collisionVisualizer = new CollisionVisualizer(collisionWorld);
		#endif
	}

	PhysicsWorld::~PhysicsWorld()
	{
		if(physicsSimulationThread!=nullptr)
		{
			interrupt();
			physicsSimulationThread->join();

			delete physicsSimulationThread;
		}

		for(unsigned int i=0; i<processables.size(); ++i)
		{
			delete processables[i];
		}

		#ifdef _DEBUG
			delete collisionVisualizer;
		#endif

		delete collisionWorld;
		delete bodyManager;
	}

	BodyManager *PhysicsWorld::getBodyManager() const
	{
		return bodyManager;
	}

	CollisionWorld *PhysicsWorld::getCollisionWorld() const
	{
		return collisionWorld;
	}

	void PhysicsWorld::addBody(AbstractBody *body)
	{
		if(body!=nullptr)
		{
			bodyManager->addBody(body);
		}
	}

	void PhysicsWorld::removeBody(AbstractBody *body)
	{
		if(body!=nullptr)
		{
			bodyManager->removeBody(body);
		}
	}

	void PhysicsWorld::addProcessable(ProcessableInterface *processable)
	{
		processables.push_back(processable);

		processable->initialize(this);
	}

	void PhysicsWorld::removeProcessable(ProcessableInterface *processable)
	{
		std::vector<ProcessableInterface *>::iterator itFind = std::find(processables.begin(), processables.end(), processable);
		if(itFind!=processables.end())
		{
			delete processable;
			processables.erase(itFind);
		}
	}

	/**
	 * @param gravity Gravity expressed in units/s^2
	 */
	void PhysicsWorld::setGravity(const Vector3<float> &gravity)
	{
		boost::recursive_mutex::scoped_lock lock(gravityMutex);

		this->gravity = gravity;
	}

	/**
	 * @return gravity expressed in units/s^2
	 */
	Vector3<float> PhysicsWorld::getGravity() const
	{
		boost::recursive_mutex::scoped_lock lock(gravityMutex);

		return gravity;
	}

	/**
	 * Launch the physics simulation in new thread
	 * @param timeStep Frequency updates expressed in second
	 * @param maxSubStep Maximum number of sub steps to execute
	 */
	void PhysicsWorld::start(float timeStep, unsigned int maxSubStep)
	{
		this->timeStep = timeStep;
		this->maxSubStep = maxSubStep;

		physicsSimulationThread = new boost::thread(std::bind(&PhysicsWorld::startPhysicsUpdate, this));
	}

	/**
	 * Interrupt the thread
	 */
	void PhysicsWorld::interrupt()
	{
		physicsSimulationStopper.store(true, std::memory_order_relaxed);
	}

	void PhysicsWorld::startPhysicsUpdate()
	{
		double remainingTime = 0.0;
		auto frameStartTime = std::chrono::high_resolution_clock::now();

		while(continueExecution())
		{
			//Compute number of sub steps to execute.
			//Note: rest of division could be used in next steps and interpolate between steps (see http://gafferongames.com/game-physics/fix-your-timestep/)
			unsigned int numSteps = static_cast<unsigned int>((timeStep - remainingTime) / timeStep);
			//Clamp number of sub steps to max sub step to avoid spiral of death (time of simulation increase and need to be executed increasingly).
			unsigned int numStepsClamped = (numSteps > maxSubStep) ? maxSubStep : numSteps;

			for(unsigned int step=0; step<numStepsClamped; ++step)
			{
				processPhysicsUpdate();
			}

			auto frameEndTime = std::chrono::high_resolution_clock::now();
			auto diffTimeMicroSeconds = std::chrono::duration_cast<std::chrono::microseconds>(frameEndTime - frameStartTime).count();
			remainingTime = timeStep - (diffTimeMicroSeconds / 1000000.0);

			if(remainingTime >= 0.0)
			{
				boost::this_thread::sleep(boost::posix_time::milliseconds(static_cast<int>(remainingTime * 1000.0)));

				remainingTime = 0.0;
				frameStartTime = std::chrono::high_resolution_clock::now();
			}else
			{
				frameStartTime = frameEndTime;
			}
		}
	}

	/**
	 * @return True if thread execution is not interrupted
	 */
	bool PhysicsWorld::continueExecution()
	{
		return !physicsSimulationStopper.load(std::memory_order_relaxed);
	}

	void PhysicsWorld::processPhysicsUpdate()
	{
		float dt = static_cast<float>(timeStep);
		Vector3<float> gravity = getGravity();

		preProcess(dt, gravity);

		collisionWorld->process(dt, gravity);

		postProcess(dt, gravity);
	}

	/**
	 * @param dt Delta of time between two simulation steps
	 * @param gravity Gravity expressed in units/s^2
	 */
	void PhysicsWorld::preProcess(float dt, const Vector3<float> &gravity)
	{
		for(unsigned int i=0; i<processables.size(); ++i)
		{
			processables[i]->setup(dt, gravity);
		}
	}

	/**
	 * @param dt Delta of time between two simulation steps
	 * @param gravity Gravity expressed in units/s^2
	 */
	void PhysicsWorld::postProcess(float dt, const Vector3<float> &gravity)
	{
		for(unsigned int i=0; i<processables.size(); ++i)
		{
			processables[i]->process(dt, gravity);
		}
	}

#ifdef _DEBUG
	const CollisionVisualizer *PhysicsWorld::getCollisionVisualizer() const
	{
		return collisionVisualizer;
	}
#endif
}
