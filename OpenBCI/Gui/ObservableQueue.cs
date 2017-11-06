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
            OnCollectionChanged(NotifyCollectionChangedAction.Remove, removed, 0);
            OnCountChanged();
            return removed;
        }
        public new void Enqueue(T item)
        {
            int index = Count;
            base.Enqueue(item);
            OnCollectionChanged(NotifyCollectionChangedAction.Add, item, index);
            OnCountChanged();
        }
        public new void Clear()
        {
            base.Clear();
            OnCollectionReset();
            OnCountChanged();
        }

        private void OnCollectionChanged(NotifyCollectionChangedAction action, object item, int index)
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(action, item, index));
        }
        private void OnCollectionReset()
        {
            CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }
        private void OnCountChanged()
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Count)));
        }
    }
}
