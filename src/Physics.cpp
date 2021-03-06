#include "Physics.h"

Physics::Physics() {

    mColor = Vector4<float>(0.0f, 1.0f, 0.0f, 1.0f);
    
    mBroadphase = new btDbvtBroadphase();

    // Set up the collision configuration and dispatcher
    mCollisionConfiguration = new btDefaultCollisionConfiguration();
    mDispatcher = new btCollisionDispatcher(mCollisionConfiguration);

    // The actual physics solver
    mSolver = new btSequentialImpulseConstraintSolver;

    // The world.
    mDynamicsWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfiguration);
    mDynamicsWorld->setGravity(btVector3(0, -5.0, 0));

    mDebugDrawer.setDebugMode(1);
    mDynamicsWorld->setDebugDrawer(&mDebugDrawer);
}


Physics::~Physics() {

    delete mBroadphase;
    delete mSolver;
    delete mDispatcher;
    delete mCollisionConfiguration;
    delete mDynamicsWorld;
}

void Physics::addGeometry(std::vector<Vector3<float> > vertList, Vector3<float> centerOfMass, unsigned int type, float volume) {

    btCollisionShape* shape;
    btMotionState* motionState;
    btScalar mass = 0;
    btVector3 inertia(0, 0, 0);

    if(type == DYNAMIC) {

        btConvexHullShape *bConvex = new btConvexHullShape();
        
        for(int i = 0; i < vertList.size(); ++i)
            bConvex->addPoint(btVector3(vertList[i][0],vertList[i][1],vertList[i][2]), false);

        bConvex->recalcLocalAabb();
        
        bConvex->setMargin(0.04); // padding, the developers claim that 0.04 is standard padding
        btShapeHull* hull = new btShapeHull(bConvex);
        btScalar margin = bConvex->getMargin();
        hull->buildHull(margin);
        
        shape = bConvex;
        mass = 1.0f;
        btVector3(centerOfMass[0], centerOfMass[1], centerOfMass[2]);
        motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(centerOfMass[0]/20.0f, centerOfMass[1]/20.0f, centerOfMass[2]/20.0f)));
        shape->calculateLocalInertia(mass, inertia);

    } else if (type == STATIC) {
        
        btConvexHullShape *bConvex = new btConvexHullShape();

        for(int i = 0; i < vertList.size(); ++i)
              bConvex->addPoint(btVector3(vertList[i][0],vertList[i][1],vertList[i][2]), false);

        bConvex->recalcLocalAabb();
        
        bConvex->setMargin(0.04); // padding
        btShapeHull* hull = new btShapeHull(bConvex);
        btScalar margin = bConvex->getMargin();
        hull->buildHull(margin);
        
        shape = new btConvexHullShape((const btScalar*)hull->getVertexPointer(), hull->numVertices(), sizeof(btVector3));
        
        motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(centerOfMass[0], centerOfMass[1], centerOfMass[2])));

    }  else {
        std::cout << "Somethign went wrong when adding physical counter part " << std::endl;
    }
    

    btRigidBody::btRigidBodyConstructionInfo shapeRigidBodyCI(mass, motionState, shape, inertia);
    btRigidBody *shapeRigidBody = new btRigidBody(shapeRigidBodyCI);
    
    shapeRigidBody->setRestitution(0.4);

    mDynamicsWorld->addRigidBody(shapeRigidBody);
    mRigidBodies.push_back(shapeRigidBody);

}

void Physics::removeGeometry(unsigned int index) {

    btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[index];
    btRigidBody* body = btRigidBody::upcast(obj);

    if (body && body->getMotionState()) {
        delete body->getMotionState();
    }

    mDynamicsWorld->removeCollisionObject( obj );
    delete obj;

    mRigidBodies.erase(mRigidBodies.begin()+index);
    mRigidBodies.shrink_to_fit();
}


void Physics::setInitialVelocity(unsigned int i, Vector3<float> v) {

    btCollisionObject* obj = mDynamicsWorld->getCollisionObjectArray()[i];
    btRigidBody* body = btRigidBody::upcast(obj);
    
    if(body && body->getMotionState()) {
        body->setLinearVelocity(btVector3(v[0], v[1], v[2]));
    }
}

void Physics::stepSimulation(Matrix4x4<float> MVP) {
    
    double deltaT = glfwGetTime() - prevTime;
    mDynamicsWorld->stepSimulation(deltaT, 10);
    prevTime = glfwGetTime();

    int numManifolds = mDynamicsWorld->getDispatcher()->getNumManifolds();
    
    for (int i=0;i<numManifolds;i++) {

        btPersistentManifold* contactManifold =  mDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        const btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        const btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());

        int numContacts = contactManifold->getNumContacts();
        
        for (int j=0;j<numContacts;j++) {
            
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            
            if (pt.getDistance()<0.f) {
            
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;
            }
        }
    }
}