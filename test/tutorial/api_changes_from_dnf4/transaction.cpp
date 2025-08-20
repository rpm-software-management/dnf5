/* includes; won't compile in tests, in the docs we leave out the comment lines to show them
#include <libdnf5/base/goal.hpp>
*/

// Create a goal
libdnf5::Goal goal(base);

// Add an RPM package named "one" for installation into the goal.
goal.add_rpm_install("one");

// Resolve the goal, create a transaction object.
auto transaction = goal.resolve();

if (transaction.get_problems() != libdnf5::GoalProblem::NO_PROBLEM) {
    for (auto message : transaction.get_resolve_logs_as_strings()) {
        std::cout << message << std::endl;
    }
}

// We can iterate over the resolved transaction and inspect the packages.
std::cout << "Resolved transaction:" << std::endl;
for (const auto & tspkg : transaction.get_transaction_packages()) {
    std::cout << tspkg.get_package().get_nevra() << ": " << transaction_item_action_to_string(tspkg.get_action())
              << std::endl;
}

// Download the packages.
transaction.download();

// Run the transaction.
transaction.run();
