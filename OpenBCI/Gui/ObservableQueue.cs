using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Gui
{
    public class ObservableQueue<T> : Queue<T>, INotifyCollectionChanged, INotifyPropertyChanged
    {
        public event NotifyCollectionChangedEventHandler CollectionChanged;
        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableQueue() : base()
        { }
        public ObservableQueue(IEnumerable<T> collection) : base(collection)
        { }
        public ObservableQueue(int capacity) : base(capacity)
        { }
        public new T Dequeue()
        {
            T removed = base.Dequeue();
            return removed;
        }
        public new void Enqueue(T item)
        {
            int index = Count;
            base.Enqueue(item);
        }
        public new void Clear()
        {
            base.Clear();
            NotifyCollectionChanged();
        }
        /// <summary>
        /// Must be called after any Enqueue/Dequeue operations
        /// </summary>
        public void NotifyCollectionChanged()
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Count)));
        }
    }
}
