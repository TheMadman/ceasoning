#include <csalt/resources.h>

#include "test_macros.h"

int main()
{
	{
		struct csalt_resource_stub stub = csalt_resource_stub(1024);

		struct csalt_resource_decorator_lazy
			lazy = csalt_resource_decorator_lazy(
				csalt_resource(&stub)
			);

		csalt_resource *resource = csalt_resource(&lazy);

		csalt_store *result = csalt_resource_init(resource);

		if (stub.init_called) {
			print_error("stub's init was called early");
			return EXIT_FAILURE;
		}

		csalt_store_read(result, 0, 10);

		if (!stub.init_called) {
			print_error("Stub's init wasn't called");
			return EXIT_FAILURE;
		}

		if (stub.return_value.last_read != 10) {
			print_error("Stub's read wasn't called");
			return EXIT_FAILURE;
		}

		csalt_store_write(result, 0, 10);

		if (stub.return_value.last_write != 10) {
			print_error("Stub's write wasn't called");
			return EXIT_FAILURE;
		}

		csalt_resource_deinit(resource);

		if (!stub.deinit_called) {
			print_error("Stub's deinit wasn't called");
			return EXIT_FAILURE;
		}
	}

	{
		struct csalt_resource_stub
			error = csalt_resource_stub_fail();

		struct csalt_resource_decorator_lazy
			lazy = csalt_resource_decorator_lazy(
				csalt_resource(&error)
			);

		csalt_resource *resource = csalt_resource(&lazy);

		csalt_store *result = csalt_resource_init(resource);

		if (!result) {
			print_error("csalt_resource_init() should have returned a store");
			return EXIT_FAILURE;
		}

		ssize_t read = csalt_store_read(result, 0, 10);

		if (read != -1) {
			print_error("csalt_store_read() should have errored");
			return EXIT_FAILURE;
		}

		ssize_t write = csalt_store_write(result, 0, 10);

		if (write != -1) {
			print_error("csalt_store_write() should have errored");
			return EXIT_FAILURE;
		}
	}
}

