# Create a goal.
goal = libdnf5.base.Goal(base)

# Add an RPM package named "one" for installation into the goal.
goal.add_rpm_install("one")

# Resolve the goal, create a transaction object.
transaction = goal.resolve()

if transaction.get_problems() != libdnf5.base.GoalProblem_NO_PROBLEM:
    for message in transaction.get_resolve_logs_as_strings():
        print(message)

# We can iterate over the resolved transaction and inspect the packages.
for tspkg in transaction.get_transaction_packages():
    print(
        tspkg.get_package().get_nevra(), ": ",
        libdnf5.base.transaction.transaction_item_action_to_string(
            tspkg.get_action()
        )
    )

# Download the packages.
transaction.download()

# Run the transaction.
transaction.run()
