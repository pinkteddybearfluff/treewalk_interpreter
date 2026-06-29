RuntimeValue FunctionCallNode::evaluateNode(shared_ptr<Environment> env) const
{
    const string& f_name = func->getIdentifierName();
    RuntimeValue obj = env->getReference(f_name).value;
    if (obj.isFunctionObj())
    {
        auto function = obj.asFunctionObj();
        auto callEnv = std::make_shared<Environment>();
        callEnv->parent = function.capturedEnv;
        if (arguments.size() == function.parameters.size())
        {
            for (int i = 0; i < arguments.size(); ++i)
            {
                callEnv->variables[function.parameters[i]] = {
                    arguments[i]->evaluateNode(env), line
                };
            }

            try
            {
                function.body->evaluateNode(callEnv);
                return {};
            }
            catch (const ReturnSignal& r)
            {
                return r.value;
            }
        }
    }
}

void FunctionDeclarationNode::evaluateNode(shared_ptr<Environment> env) const
{
    auto* body_ptr = dynamic_cast<BlockNode*>(body.get());
    env->declare(identifier, {FunctionObject{parameters, body_ptr, env}, line});
}

struct FunctionObject
{
    vector<string> parameters;
    const BlockNode* body;
    shared_ptr<Environment> capturedEnv;
};

struct Environment
{
    std::map<string, VariableInfo> variables;
    shared_ptr<Environment> parent;
    VariableInfo& getReference(const string& identifier);
    void declare(string name, VariableInfo data);
};

VariableInfo& Environment::getReference(const string& identifier)
{
    const auto& iter = variables.find(identifier);
    if (iter != variables.end())
    {
        return iter->second;
    }
    if (parent)
    {
        return parent->getReference(identifier);
    }
    throw UndefinedVariable();
}

void Environment::declare(string name, VariableInfo data)
{
    auto iter = variables.find(name);


    if (iter == variables.end())
        variables[name] = data;
    else throw Redeclaration();
}
