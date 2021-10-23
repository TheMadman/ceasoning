#include <csalt/resources.h>

#include "test_macros.h"

int main()
{
	{
		struct csalt_resource_stub stub = csalt_resource_stub(1024);
		struct csalt_resource_stub stub_2 = csalt_resource_stub(512);

		csalt_resource *resources[] = {
			csalt_resource(&stub),
			csalt_resource(&stub_2),
		};

		struct csalt_resource_pair pairs[arrlength(resources)] = { 0 };

		int error = csalt_resource_pair_list(resources, pairs);

		if (error) {
			print_error("Error creating pairs");
			return EXIT_FAILURE;
		}

		csalt_store
			*store = csalt_resource_init(csalt_resource(pairs));

		struct csalt_store_pair *result = (void *)store;

		if ((void *)result->first != &stub.return_value) {
			print_error(
				"Unexpected first return value: %p -> %p",
				result->first,
				&stub.return_value
			);
			return EXIT_FAILURE;
		}

		struct csalt_store_pair *second = (void *)result->second;

		if ((void *)second->first != &stub_2.return_value) {
			print_error(
				"Unexpected second return value: %p -> %p",
				second->first,
				&stub_2.return_value
			);
			return EXIT_FAILURE;
		}

		csalt_resource_deinit(csalt_resource(pairs));

		if (!stub.deinit_called) {
			print_error(
				"csalt_resource_deinit() not called on stub"
			);
			return EXIT_FAILURE;
		}

		if (!stub_2.deinit_called) {
			print_error(
				"csalt_resource_deinit() not called on stub_2"
			);
			return EXIT_FAILURE;
		}
	}
}
